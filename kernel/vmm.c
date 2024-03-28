#include <type.h>
#include <sys.h>
#include <list.h>
#include <kmalloc.h>
#include <panic.h>
#include <page.h>
#include <pmm.h>

struct vmm_node {
    u32 end;
    u32 size;
    struct list_head list;
};

struct vmm {
    u32 vmm_bottom;
    u32 vmm_top;
    struct list_head free_list;
    struct list_head alloc_list;
};

static struct vmm vmm;

void vmm_init()
{
    vmm.vmm_top = vmm.vmm_bottom = ALLOC_REGION_TOP;
    list_init_head(&vmm.free_list);
    list_init_head(&vmm.alloc_list);
}

static struct vmm_node* vmm_find_node_fit(struct list_head* head, u32 size)
{
    struct vmm_node* node;
    list_for_each_entry(node, head, list) {
        if(node->size >= size) {
            return node;
        }
    }
    return NULL;
}

static struct vmm_node* vmm_find_node_at(struct list_head* head, u32 vaddr)
{
    struct vmm_node* node;
    list_for_each_entry(node, head, list) {
        if(node->end - node->size == vaddr) {
            return node;
        }
    }
    return NULL;
}

static struct vmm_node* vmm_find_node_left(struct list_head* head, struct vmm_node* node)
{
    struct vmm_node* np;
    list_for_each_entry(np, head, list) {
        if(np->end == node->end - node->size) {
            return np;
        }
    }
    return NULL;
}

static struct vmm_node* vmm_find_node_right(struct list_head* head, struct vmm_node* node)
{
    struct vmm_node* np;
    list_for_each_entry(np, head, list) {
        if(np->end - np->size == node->end) {
            return np;
        }
    }
    return NULL;
}

static inline void vmm_insert_node(struct list_head* head, struct vmm_node* node)
{
    list_add_tail(&node->list, head);
}

static inline void vmm_insert_node_sort_size(struct list_head* head, struct vmm_node* node)
{
    struct vmm_node* np;
    list_for_each_entry(np, head, list) {
        if(np->size > node->size) {
            break;
        }
    }
    list_add_tail(&node->list, &np->list);
}

static void* vmm_map(struct vmm_node* node)
{
    u32 vaddr = node->end - node->size;
    for(int i = 0; i < node->size; i += PAGE_SIZE) {
        uintptr_t frame = pmm_alloc(1);
        page_map(vaddr + i, frame, KERNEL_PAGE_FLAGS);
    }

    return (void*)vaddr;
}

static void vmm_unmap(struct vmm_node* node)
{
    u32 vaddr = node->end - node->size;
    for(int i = 0; i < node->size; i += PAGE_SIZE) {
        page_unmap(vaddr + i);
    }
}

void* vmm_alloc(u32 size)
{
    assert(size % PAGE_SIZE == 0, "vmm_alloc: size must be multiple of PAGE_SIZE");

    // allocated from free lists
    struct vmm_node* fnode = vmm_find_node_fit(&vmm.free_list, size);

    if(fnode) {
        struct vmm_node* alloc_node;
        if(fnode->size > size) {
            alloc_node = (struct vmm_node*)kmalloc(sizeof(struct vmm_node));
            alloc_node->end = fnode->end;
            alloc_node->size = size;
            fnode->end -= size;
            fnode->size -= size;
        } else {
            alloc_node = fnode;
            list_del(&fnode->list);
        }

        vmm_insert_node(&vmm.alloc_list, alloc_node);
        return vmm_map(alloc_node);
    }

    // allocated from stack top

    // [todo]: check top boundary
    // if(vmm_top - mark < size) {
    //     panic("vmm_alloc: out of memory");
    // }

    struct vmm_node* alloc_node = (struct vmm_node*)kmalloc(sizeof(struct vmm_node));
    alloc_node->end = vmm.vmm_top;
    alloc_node->size = size;
    vmm_insert_node(&vmm.alloc_list, alloc_node);
    vmm.vmm_top -= size;
    return vmm_map(alloc_node);
}

void vmm_free(void* vaddr)
{
    struct vmm_node* alloc_node = vmm_find_node_at(&vmm.alloc_list, (u32)vaddr);

    if(alloc_node == NULL) {
        panic("vmm_free: invalid address");
    }

    vmm_unmap(alloc_node);

    list_del(&alloc_node->list);

    struct vmm_node* free_left = vmm_find_node_left(&vmm.free_list, alloc_node);
    if(free_left) {
        list_del(&free_left->list);
        alloc_node->size += free_left->size;
        kfree(free_left);
    }

    struct vmm_node* free_high = vmm_find_node_right(&vmm.free_list, alloc_node);
    if(free_high) {
        list_del(&free_high->list);
        free_high->size += alloc_node->size;
        kfree(alloc_node);
        alloc_node = free_high;
    }

    if(alloc_node->end - alloc_node->size == vmm.vmm_top) {
        vmm.vmm_top = alloc_node->end;
        kfree(alloc_node);
    } else {
        vmm_insert_node_sort_size(&vmm.free_list, alloc_node);
    }
}
