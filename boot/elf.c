#include <utils.h>
#include <type.h>
#include <mem.h>
#include <boot.h>

struct elf_header {
    u8 magic[4];
    u8 class;
    u8 endian;
    u8 version;
    u8 abi;
    u8 abi_version;
    u8 reserve[7];
    u16 type;
    u16 machine;
    u32 version2;
    u32 entry;
    u32 phoff;
    u32 shoff;
    u32 flags;
    u16 ehsize;
    u16 phentsize;
    u16 phnum;
    u16 shentsize;
    u16 shnum;
    u16 shstrndx;
} __attribute__((packed));

struct elf_prog_header {
    u32 type;
    u32 offset;
    u32 vaddr;
    u32 paddr;
    u32 filesz;
    u32 memsz;
    u32 flags;
    u32 align;
} __attribute__((packed));

static void elf_print_head(struct elf_header* elf_header)
{
    printf("ELF Header:\n");
    printf("  Magic:   ");
    for (int i = 0; i < 4; i++) {
        printf("%x ", elf_header->magic[i]);
    }
    printf("\n");
    printf("  Class:                             %x\n", elf_header->class);
    printf("  Data:                              %x\n", elf_header->endian);
    printf("  Version:                           %x\n", elf_header->version);
    printf("  OS/ABI:                            %x\n", elf_header->abi);
    printf("  ABI Version:                       %x\n", elf_header->abi_version);
    printf("  Type:                              %x\n", elf_header->type);
    printf("  Machine:                           %x\n", elf_header->machine);
    printf("  Version:                           %x\n", elf_header->version2);
    printf("  Entry point address:               %x\n", elf_header->entry);
    printf("  Start of program headers:          %x\n", elf_header->phoff);
    printf("  Start of section headers:          %x\n", elf_header->shoff);
    printf("  Flags:                             %x\n", elf_header->flags);
    printf("  Size of this header:              %x\n", elf_header->ehsize);
}

static void elf_print_prog(struct elf_prog_header* elf_prog_header)
{
    printf("Program Header:\n");
    printf("  Type:                              %x\n", elf_prog_header->type);
    printf("  Offset:                            %x\n", elf_prog_header->offset);
    printf("  Virtual Address:                   %x\n", elf_prog_header->vaddr);
    printf("  Physical Address:                  %x\n", elf_prog_header->paddr);
    printf("  File Size:                         %x\n", elf_prog_header->filesz);
    printf("  Memory Size:                       %x\n", elf_prog_header->memsz);
    printf("  Flags:                             %x\n", elf_prog_header->flags);
    printf("  Align:                             %x\n", elf_prog_header->align);
}


// ugly code, but it works
static int elf_is_vaild(struct elf_header* elf_header)
{
    if (elf_header->magic[0] != 0x7f || elf_header->magic[1] != 'E' || elf_header->magic[2] != 'L' || elf_header->magic[3] != 'F') {
        printf("Not a vaild ELF file!\n");
        return 0;
    }
    if (elf_header->class != 1) {
        printf("Not a 32-bit ELF file!\n");
        return 0;
    }
    if (elf_header->endian != 1) {
        printf("Not a little endian ELF file!\n");
        return 0;
    }
    if (elf_header->version != 1) {
        printf("Not a vaild ELF file!\n");
        return 0;
    }
    if (elf_header->abi != 0) {
        printf("Not a vaild ELF file!\n");
        return 0;
    }
    if (elf_header->abi_version != 0) {
        printf("Not a vaild ELF file!\n");
        return 0;
    }
    if (elf_header->type != 2) {
        printf("Not a executable ELF file!\n");
        return 0;
    }
    if (elf_header->machine != 3) {
        printf("Not a i386 ELF file!\n");
        return 0;
    }
    if (elf_header->version2 != 1) {
        printf("Not a vaild ELF file!\n");
        return 0;
    }
    elf_print_head(elf_header);
    return 1;
}

u32 elf_get_mem_size(char* file)
{
    u32 start = 0xffffffff;
    u32 end = 0;

    int prog_num = ((struct elf_header*)file)->phnum;
    struct elf_prog_header* ph_table = (struct elf_prog_header*)(file + ((struct elf_header*)file)->phoff);

    for(int i = 0; i < prog_num; i++) {
        if(ph_table[i].type != 1)
            continue;

        if(ph_table[i].vaddr < start)
            start = ph_table[i].vaddr;

        if(ph_table[i].vaddr + ph_table[i].memsz > end)
            end = ph_table[i].vaddr + ph_table[i].memsz;
    }

    return ALIGN_UP((end - start), PAGE_SIZE);
}

#define KERNEL_VADDR_BASE 0xc0000000

// setup kernel from the elf file
u32 elf_load_kernel(char* file, u32 load_paddr)
{
    if(!elf_is_vaild((struct elf_header*)file)) {
        return 0;
    }

    u32 pv_off = KERNEL_VADDR_BASE - load_paddr;

    int prog_num = ((struct elf_header*)file)->phnum;
    struct elf_prog_header* ph_table = (struct elf_prog_header*)(file + ((struct elf_header*)file)->phoff);
    
    // load all loadable program segments into memory
    for(int i = 0; i < prog_num; i++) {
        // elf_print_prog(&ph_table[i]);

        if(ph_table[i].type != 1)
            continue;
        
        bool writeable = ph_table[i].flags & 0x2 ? true : false;
        memcpy(ph_table[i].vaddr - pv_off, file + ph_table[i].offset, ph_table[i].filesz);
        page_map_kernel_pages(ph_table[i].vaddr - pv_off, ph_table[i].vaddr, ph_table[i].memsz, writeable);
    }

    return ((struct elf_header*)file)->entry;
}