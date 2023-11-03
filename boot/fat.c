#include <boot.h>
#include <type.h>
#include <mem.h>

static u8 buffer[512] = {0};

enum fat_type {
    UNKONWN,
    FAT_12,
    FAT_16,
    FAT_32,
};

struct fat {
    u32 parti_id;

    u32 fat_sec;
    u16 fat_num;

    u16 clus_sec;
    u32 clus_num;
    
    u32 first_data_sec;

    enum fat_type type;
} fat;

struct bpb_common {
    u8 jmp[3];
    u8 oem[8];
    u16 sec_byte;
    u8 clus_sec;
    u16 res_sec;
    u8 fat_num;
    u16 root_ent_num;
    u16 sec_num_16;
    u8 media;
    u16 fat_sec;
    u16 trk_sec;
    u16 head_num;
    u32 hidden_sec;
    u32 sec_num_32;
} __attribute__((packed));

struct bpb_16 {
    struct bpb_common common;
    u8 drv_num;
    u8 reserved;
    u8 boot_sig;
    u32 vol_id;
    u8 vol_lab[11];
    u8 fs_type[8];
} __attribute__((packed));

struct bpb_32 {
    struct bpb_common common;
    u32 fat_sec;
    u16 ext_flag;
    u16 fs_ver;
    u32 root_clus;
    u16 fs_info;
    u16 boot_sec;
    u8 reserved[12];
    u8 drv_num;
    u8 reserved1;
    u8 boot_sig;
    u32 vol_id;
    u8 vol_lab[11];
    u8 fs_type[8];
} __attribute__((packed));


static inline u32 fat_get_sec_clus(u32 clus)
{
    return fat.first_data_sec + (clus - 2) * fat.clus_sec;
}

void fat_init(int parti_id)
{
    disk_read(parti_id, (void*)buffer, 0, 1);
    struct bpb_common* bpb_common = (struct bpb_common*)buffer;

    u32 root_dir_sec = ((bpb_common->root_ent_num * 32) + (bpb_common->sec_byte - 1)) / bpb_common->sec_byte;
    
    u32 fat_sec;
    if(bpb_common->fat_sec != 0) 
        fat_sec = bpb_common->fat_sec;
    else
        fat_sec = ((struct bpb_32*)bpb_common)->fat_sec;

    u32 total_sec;
    if(bpb_common->sec_num_16 != 0)
        total_sec = bpb_common->sec_num_16;
    else
        total_sec = bpb_common->sec_num_32;

    u32 data_sec = total_sec - (bpb_common->res_sec + bpb_common->fat_num * fat_sec + root_dir_sec);
    u32 clus_num = data_sec / bpb_common->clus_sec;
    if(clus_num < 4085) {
        printf("fat12 is not supported!\n");
        return;
    } else if(clus_num < 65525) {
        printf("fat16 detected!\n");
        fat.type = FAT_16;
    } else {
        printf("fat32 detected!\n");
        fat.type = FAT_32;
    }
    
    fat.parti_id = parti_id;
    fat.clus_sec = bpb_common->clus_sec;
    fat.clus_num = clus_num;
    fat.fat_sec = fat_sec;
    fat.fat_num = bpb_common->fat_num;
    fat.first_data_sec = bpb_common->res_sec + bpb_common->fat_num * fat_sec + root_dir_sec;
}