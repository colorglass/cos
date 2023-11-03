#include <utils.h>
#include <type.h>
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

void elf_print_head(void* buffer)
{
    struct elf_header* elf_header = (struct elf_header*)buffer;
    printf("ELF Header:\n");
    printf("  Magic:   ");
    for (int i = 0; i < 4; i++)
    {
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