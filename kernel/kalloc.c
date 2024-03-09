#include <type.h>
#include <sys.h>
#include <mem.h>
#include <list.h>

// the max alloc chunk size is 2^14 --> 16KB
#define KHEAP_BIN_COUNT 13
#define KHEAP_MIN_ALLOC sizeof(struct list_head)
#define KHEAP_MAX_ALLOC (1 << 14)

#define KHEAP_MIN_NODE_SIZE (sizeof(struct knode) + sizeof(struct footer))
#define KHEAP_NODE_OVERHEAD (offset_of(struct knode, list) + sizeof(struct footer))

#define KHEAP_MIN_WILD_SIZE 0x1000
#define KHEAP_MAX_FRAGMENTS 8

#define PRE_BUFFER_SIZE 0x100000
static u8 pre_buffer[PRE_BUFFER_SIZE] __attribute__((aligned(PAGE_SIZE)));

struct kheap
{
    void *start;
    void *end;
    struct list_head free_bins[KHEAP_BIN_COUNT];
    int free_counts[KHEAP_BIN_COUNT];
};

struct knode
{
    u32 size;
    bool free;
    struct list_head list;
} __attribute__((packed));

struct footer
{
    u32 size;
};

static struct kheap kheap;

static inline int kheap_expand()
{
    return 0;
}

static inline struct knode *kheap_get_wild(struct kheap *heap)
{
    struct footer *footer = (struct footer *)((u8 *)heap->end - sizeof(struct footer));
    return (struct knode *)((u8 *)heap->end - footer->size - KHEAP_NODE_OVERHEAD);
}

static inline struct footer *knode_get_footer(struct knode *node)
{
    return (struct footer *)((u8 *)node + node->size + offset_of(struct knode, list));
}

static inline void knode_make_footer(struct knode *node)
{
    struct footer *footer = knode_get_footer(node);
    footer->size = node->size;
}

static int bin_get_index(u32 size)
{
    int index = 0;
    size = size < KHEAP_MIN_ALLOC ? KHEAP_MIN_ALLOC : size;
    while (size >>= 1)
        index++;
    index -= 2;

    index = index >= KHEAP_BIN_COUNT ? KHEAP_BIN_COUNT - 1 : index;
    return index;
}

static struct knode *free_bins_best_fit(struct list_head *head, u32 size)
{
    struct knode *entry = NULL;
    list_for_each_entry(entry, head, list)
    {
        if (entry->size >= size)
            return entry;
    }
    return NULL;
}

static void add_free_node(struct knode *node)
{
    int index = bin_get_index(node->size);
    struct list_head *head = &kheap.free_bins[index];
    struct knode *entry = NULL;
    list_for_each_entry(entry, head, list)
    {
        if (entry->size > node->size)
        {
            break;
        }
    }

    list_add_tail(&node->list, &entry->list);
    kheap.free_counts[index]++;
}

static void remove_free_node(struct knode *node)
{
    int index = bin_get_index(node->size);
    list_del_init(&node->list);
    kheap.free_counts[index]--;
    if (kheap.free_counts[index] < 0)
    {
        panic("free_counts is negative\n");
    }
}

void *kalloc(u32 size)
{
    if (size == 0 || size > KHEAP_MAX_ALLOC)
        return NULL;

    int index = bin_get_index(size);
    struct knode *best_fit = free_bins_best_fit(&kheap.free_bins[index], size);
    while (best_fit == NULL)
    {
        if (index + 1 >= KHEAP_BIN_COUNT)
            return NULL;
        best_fit = free_bins_best_fit(&kheap.free_bins[++index], size);
    }

    // if there is enough space to split
    if (best_fit->size - size > KHEAP_MIN_NODE_SIZE)
    {
        // create new node
        struct knode *new_node = (struct knode *)((u8 *)best_fit + size + KHEAP_NODE_OVERHEAD);
        new_node->size = best_fit->size - size - KHEAP_NODE_OVERHEAD;
        knode_make_footer(new_node);
        add_free_node(&kheap.free_bins[bin_get_index(new_node->size)], &new_node->list);

        best_fit->size = size;
        knode_make_footer(best_fit);
    }

    best_fit->free = false;
    remove_free_node(best_fit);
    struct knode *wilder = kheap_get_wild(&kheap);
    if (wilder->size < KHEAP_MIN_WILD_SIZE)
    {
        int err = kheap_expand();
        if (err)
            return NULL;
    }

    return (void *)((u8 *)best_fit + offset_of(struct knode, list));
}

void kfree(void *ptr)
{
    if (ptr == NULL)
        return;

    struct knode *node = (struct knode *)((u8 *)ptr - offset_of(struct knode, list));

    if (node->free == true)
        return;

    int index = bin_get_index(node->size);
    if (kheap.free_counts[index] > KHEAP_MAX_FRAGMENTS)
    {
        struct knode *next = (struct knode *)((u8 *)node + node->size + KHEAP_NODE_OVERHEAD);

        if (next < kheap.end && next->free)
        {
            node->size = node->size + next->size + KHEAP_NODE_OVERHEAD;
            struct footer *old_footer = knode_get_footer(node);
            knode_make_footer(node);
            memset(next, 0, sizeof(struct knode));
            memset(old_footer, 0, sizeof(struct footer));
            remove_free_node(next);
        }

        if (node > kheap.start)
        {
            struct footer *prev_footer = (struct footer *)((u8 *)node - sizeof(struct footer));
            struct knode *prev = (struct knode *)((u8 *)node - prev_footer->size - KHEAP_NODE_OVERHEAD);
            if (prev->free)
            {
                prev->size = prev->size + node->size + KHEAP_NODE_OVERHEAD;
                knode_make_footer(prev);
                memset(node, 0, sizeof(struct knode));
                memset(prev_footer, 0, sizeof(struct footer));
                remove_free_node(prev);
                node = prev;
            }
        }
    }
    
    node->free = true;
    add_free_node(node);
}

int kalloc_init()
{
    for (int i = 0; i < KHEAP_BIN_COUNT; i++)
    {
        kheap.free_counts[i] = 0;
        list_init_head(&kheap.free_bins[i]);
    }

    kheap.start = (void *)pre_buffer;
    kheap.end = (void *)(pre_buffer + PRE_BUFFER_SIZE);

    struct knode *init_region = (struct knode *)pre_buffer;
    // the overhead memory are knode.size and footer, since knode.list is unused for the allocated chunk
    init_region->size = PRE_BUFFER_SIZE - KHEAP_NODE_OVERHEAD;
    init_region->free = true;
    knode_make_footer(init_region);
    add_free_node(init_region);

    return 0;
}
