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

    u8 parti_id;
    u32 fat_sec;
    u16 fat_num;

    u16 sec_byte;
    u16 clus_sec;
    u32 clus_num;
    
    u32 first_data_sec;
    u32 first_fat_sec;

    enum fat_type type;

    u32 root_dir_sec[32];
    u32 root_dir_sec_num;
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

struct dir_entry {
    u8 name[11];
    u8 attr;
    u8 res;
    u8 crt_time_tenth;
    u16 crt_time;
    u16 crt_date;
    u16 last_acc_date;
    u16 fst_clus_hi;
    u16 wrt_time;
    u16 wrt_date;
    u16 fst_clus_lo;
    u32 file_size;
}__attribute__((packed));

static inline u32 fat_get_data_sec(u32 clus)
{
    return fat.first_data_sec + (clus - 2) * fat.clus_sec;
}

// return fat sector number from cluster number, ent_off return the offset of cluster entry in fat sector
static u32 fat_get_fat_sec_ent(struct fat* fat, u32 clus, u32* ent_off)
{
    u32 fat_offset;
    if(fat->type == FAT_16)
        fat_offset = clus * 2;
    else if(fat->type == FAT_32)
        fat_offset = clus * 4;
    else 
        return 0;
    

    if(!ent_off)
        return 0;

    *ent_off = fat_offset % fat->sec_byte;
    return fat->first_fat_sec + fat_offset / fat->sec_byte;
}

// return the next chaining cluster number
static u32 fat_get_next_clus(struct fat* fat, u32 clus)
{
    u32 fat_sec, ent_off;
    fat_sec = fat_get_fat_sec_ent(fat, clus, &ent_off);
    if(!fat_sec)
        return 0;
    
    disk_read(fat->parti_id, (void*)buffer, fat_sec, 1);

    if(fat->type == FAT_16)
        return *(u16*)&buffer[ent_off];     // may be force cast to u32
    else if(fat->type == FAT_32)
        return *(u32*)&buffer[ent_off] & 0x0fffffff;
    else
        return 0;
}

static int fat_clus_is_end(struct fat* fat, u32 clus)
{
    if(fat->type == FAT_16)
        return clus >= 0xfff8;
    else if(fat->type == FAT_32)
        return clus >= 0x0ffffff8;
    else
        return 0;
}

// init fat file system, and there is only one fat instance for now
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
    fat.sec_byte = bpb_common->sec_byte;
    fat.first_fat_sec = bpb_common->res_sec;
    fat.first_data_sec = bpb_common->res_sec + bpb_common->fat_num * fat_sec + root_dir_sec;


    // get the root director related sectors for the next kernel read
    if(fat.type == FAT_16) {
        // the root directory is in the fixed location in fat16
        fat.root_dir_sec_num = root_dir_sec;
        for(int i=0; i < root_dir_sec; i++) {
            fat.root_dir_sec[i] = fat.first_fat_sec + fat.fat_num * fat_sec + i;
        }
    } else if (fat.type == FAT_32) {
        fat.root_dir_sec_num = 0;
        u32 root_clus = ((struct bpb_32*)bpb_common)->root_clus;

        // traverse the root directory cluster chain
        do {
            // we find the cluster, and now we add all the sectors of the cluster into array
            for(int i=0; i< fat.clus_sec; i++) {
                fat.root_dir_sec[fat.root_dir_sec_num++] = fat_get_data_sec(root_clus) + i;
            }

            // next cluster
            root_clus = fat_get_next_clus(&fat, root_clus);
        } while(!fat_clus_is_end(&fat, root_clus));
    }
}

void fat_print_fat()
{
    //print fat struct attribute
    printf("fat struct attribute:\n");
    printf("  fat_sec: %d\n", fat.fat_sec);
    printf("  fat_num: %d\n", fat.fat_num);
    printf("  sec_byte: %d\n", fat.sec_byte);
    printf("  clus_sec: %d\n", fat.clus_sec);
    printf("  clus_num: %d\n", fat.clus_num);
    printf("  first_data_sec: %d\n", fat.first_data_sec);
    printf("  first_fat_sec: %d\n", fat.first_fat_sec);
    printf("  type: %d\n", fat.type);
}

// load the kernel file into memory, the kernel file name is KERNEL.ELF and it should be in the root directory
u32 fat_load_kernel(void* dest)
{
    // kernel file name string in the typical directory entry of fat.
    // it should be KERNEL.ELF in file system
    const char *kernel_name = "KERNEL  ELF";
    u32 kernel_clus = 0;
    u32 kernel_size = 0;

    // traverse the root directory to find the kernel file
    u32 dir_ent_num = fat.sec_byte / sizeof(struct dir_entry);      // dir entry count in one sector
    for(int i = 0; i < fat.root_dir_sec_num; i++) {
        disk_read(fat.parti_id, (void*)buffer, fat.root_dir_sec[i], 1);

        struct dir_entry* dir_entry = (struct dir_entry*)buffer;
        
        for(int j = 0; j < dir_ent_num; j++) {
            // not a vaild entry
            if( dir_entry[j].attr == 0xe5)
                continue;

            // no more vaild entry
            if( dir_entry[j].attr == 0x00)
                break;

            if(memcmp(dir_entry[j].name, kernel_name, 11) == 0) {
                kernel_clus = (dir_entry[j].fst_clus_lo & 0xffff) | (dir_entry[j].fst_clus_hi << 16);
                kernel_size = dir_entry[j].file_size;
                goto find_kernel;
            }
        }
    }

    if(kernel_clus == 0)
        return 0;

find_kernel:
    u32 left_bytes = kernel_size;
    u32 copy_bytes = 0;

    // read kernel file from the disk
    do {
        disk_read(fat.parti_id, (void*)buffer, fat_get_data_sec(kernel_clus), 1);

        copy_bytes = left_bytes > fat.sec_byte ? fat.sec_byte : left_bytes;
        memcpy(dest, buffer, copy_bytes);
        dest += copy_bytes;
        left_bytes -= copy_bytes;

        kernel_clus = fat_get_next_clus(&fat, kernel_clus);
    }while(!fat_clus_is_end(&fat, kernel_clus));
    
    return left_bytes ? 0 : kernel_size;
}