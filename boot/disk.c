#include <boot.h>
#include <type.h>

#define MBR_PARTI_SIZE 4

struct parti_entry
{
    u8 bootable;
    u8 chs_first[3];
    u8 type;
    u8 chs_end[3];
    u32 lba_first;
    u32 sec_count;
} __attribute__((packed));

struct partition
{
    u8 type;
    u8 bootable;
    u32 sec_start;
    u32 sec_count;
};

struct disk
{
    u32 signature;
    u32 partion_num;
    u64 sec_count;

    struct partition partitions[MBR_PARTI_SIZE];
};

extern const u8 _mbr[];
static struct disk disk;

int disk_init()
{
    int partition_num = 0;
    u64 sec_count = 0;
    u32 *mbr_sign = (u32 *)&_mbr[0x1b8];
    disk.signature = *mbr_sign;

    struct parti_entry *parti = (struct parti_entry *)&_mbr[0x1be];
    for (int i = 0; i < MBR_PARTI_SIZE; i++)
    {
        if (parti[i].type == 0)
            continue;

        disk.partitions[partition_num].type = parti[i].type;
        disk.partitions[partition_num].bootable = parti[i].bootable & 0x80 ? 1 : 0;
        disk.partitions[partition_num].sec_start = parti[i].lba_first;
        disk.partitions[partition_num].sec_count = parti[i].sec_count;
        sec_count += parti[i].sec_count;
        partition_num++;
    }

    disk.partion_num = partition_num;
    disk.sec_count = sec_count;
    return 0;
}

void disk_print_info()
{
    printf("Disk signature: 0x%x\n", disk.signature);
    printf("Disk partition number: %d\n", disk.partion_num);
    printf("Disk sector count: %d\n", disk.sec_count);
    printf("Disk size: %dMb\n", (disk.sec_count * SECTOR_SIZE) / 0x100000);
    for (int i = 0; i < disk.partion_num; i++)
    {
        printf("Partition %d:\n", i);
        printf("    Type: 0x%x\n", disk.partitions[i].type);
        printf("    Bootable: %s\n", disk.partitions[i].bootable ? "Yes" : "No");
        printf("    Sector start: %d\n", disk.partitions[i].sec_start);
        printf("    Sector count: %d\n", disk.partitions[i].sec_count);
        printf("    Size: %dMb\n", (disk.partitions[i].sec_count * SECTOR_SIZE) / 0x100000);
    }
}

int disk_find_next_bootable(int next)
{
    for (int i = next; i < disk.partion_num; i++)
    {
        if (disk.partitions[i].bootable)
            return i;
    }
    return -1;
}

int disk_read(int parti_id, void* dest, u64 sec_start, u16 sec_count)
{
    if(parti_id >= disk.partion_num)
        return -1;

    struct partition *partition;
    partition = &disk.partitions[parti_id];

    if(sec_start + sec_count > partition->sec_count)
        return -1;

    ide_pio_read(dest, sec_start + partition->sec_start, sec_count);
    return 0;
}

int disk_write(int parti_id, void* src, u32 size, u64 sec_start)
{
    if(parti_id >= disk.partion_num)
        return -1;

    struct partition *partition;
    partition = &disk.partitions[parti_id];

    /* 512 bytes per write, be sure the write buffer size is 512 bytes multiple */
    u16 sec_count = (size + SECTOR_SIZE - 1) / SECTOR_SIZE;
    if(sec_start + sec_count > partition->sec_count)
        return -1;

    ide_pio_write(src, sec_start + partition->sec_start, sec_count);
    return 0;
}