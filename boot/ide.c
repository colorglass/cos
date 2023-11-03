#include <type.h>
#include <utils.h>
#include <mem.h>
#include <boot.h>

extern int printf(const char *str, ...);

#define INVAILD 0
#define VAILD 1

#define IDE0_BASE 0x1F0
#define IDE0_CTRL 0x3F6
#define IDE1_BASE 0x170
#define IDE1_CTRL 0x376

/* normal register sets offset */
#define IDE_REG_DATA 0
#define IDE_REG_ERR 1
#define IDE_REG_CNT 2
#define IDE_REG_LBAL 3
#define IDE_REG_LBAM 4
#define IDE_REG_LBAH 5
#define IDE_REG_DRI 6
#define IDE_REG_STA 7
#define IDE_REG_CMD 7

/* control register sets offset */
#define IDE_REG_ASTA 0
#define IDE_REG_CTRL 0
#define IDE_REG_ADDR 1

/* error register bits */
#define IDE_ERR_AMNF BIT(0)
#define IDE_ERR_TK0NF BIT(1)
#define IDE_ERR_ABRT BIT(2)
#define IDE_ERR_MCR BIT(3)
#define IDE_ERR_IDNF BIT(4)
#define IDE_ERR_MC BIT(5)
#define IDE_ERR_UNC BIT(6)
#define IDE_ERR_BBK BIT(7)

/* drive / head register bits */
#define IDE_DRI_SEC BIT(4)
#define IDE_DRI_LBA BIT(6)

/* status register bits */
#define IDE_STA_ERR BIT(0)
#define IDE_STA_IDX BIT(1)
#define IDE_STA_CORR BIT(2)
#define IDE_STA_DRQ BIT(3)
#define IDE_STA_SRV BIT(4)
#define IDE_STA_DF BIT(5)
#define IDE_STA_RDY BIT(6)
#define IDE_STA_BSY BIT(7)

#define IDE_CTL_NIEN BIT(1)
#define IDE_CTL_SRST BIT(2)
#define IDE_CTL_HOB BIT(7)

struct ide_device
{
    u8 bus;
    u8 select;
    u16 io_base;
    u16 ctrl_base;
};

// static u16 identity[256];

static struct ide_device ide_device = {0, 0, IDE0_BASE, IDE0_CTRL};

static inline void ide_select(struct ide_device *ide_dev)
{
    /* It should be LBA supported */
    outb(ide_dev->io_base + IDE_REG_DRI, 0xe0 | ide_dev->select << 4);
}

static inline void ide_disable_irq(struct ide_device *ide_dev)
{
    outb(ide_dev->ctrl_base + IDE_REG_CTRL, IDE_CTL_NIEN);
}

static inline void ide_flush_cache(struct ide_device *ide_dev)
{
    outb(ide_dev->io_base + IDE_REG_CMD, 0xea);
    while(inb(ide_dev->io_base + IDE_REG_STA) & IDE_STA_BSY);
}

static int ide_detect(struct ide_device *ide_dev)
{
    ide_select(ide_dev);

    inb(ide_dev->io_base + IDE_REG_STA);
    inb(ide_dev->io_base + IDE_REG_STA);
    inb(ide_dev->io_base + IDE_REG_STA);
    inb(ide_dev->io_base + IDE_REG_STA);
    while (inb(ide_dev->io_base + IDE_REG_STA) & IDE_STA_BSY)
        ;

    outb(ide_dev->io_base + IDE_REG_CMD, 0xec);

    if (!inb(ide_dev->io_base + IDE_REG_STA))
        return INVAILD;

    while((inb(ide_dev->io_base + IDE_REG_STA) ^ IDE_STA_DRQ) & (IDE_STA_BSY | IDE_STA_DRQ));

    for (int i = 0; i < SECTOR_SIZE / 2; i++)
    {
        // identity[i] = inw(ide_dev->io_base + IDE_REG_DATA);
        inw(ide_dev->io_base + IDE_REG_DATA);
    }

    return VAILD;
}

int ide_init()
{
    if (ide_detect(&ide_device) == INVAILD)
    {
        printf("IDE device not found!\n");
        return -1;
    }
    ide_disable_irq(&ide_device);
    return 0;
}

u32 ide_pio_read(void* dest, u64 sec_start, u16 sec_count)
{
    u16 *buffer = (u16 *)dest;
    u32 index = 0;
    u16 sec_start_hi = sec_start >> 32;
    u32 sec_start_lo = sec_start;


    outb(ide_device.io_base + IDE_REG_CNT, (sec_count >> 8) & 0xff);
    outb(ide_device.io_base + IDE_REG_LBAH, (sec_start_hi >> 8) & 0xff);
    outb(ide_device.io_base + IDE_REG_LBAM, sec_start_hi & 0xff);
    outb(ide_device.io_base + IDE_REG_LBAL, (sec_start_lo >> 24) & 0xff);
    outb(ide_device.io_base + IDE_REG_CNT, sec_count && 0xff);
    outb(ide_device.io_base + IDE_REG_LBAH, (sec_start_lo >> 16) & 0xff);
    outb(ide_device.io_base + IDE_REG_LBAM, (sec_start_lo >> 8) & 0xff);
    outb(ide_device.io_base + IDE_REG_LBAL, sec_start_lo & 0xff);

    outb(ide_device.io_base + IDE_REG_CMD, 0x24);

    while (sec_count--)
    {
        while ((inb(ide_device.io_base + IDE_REG_STA) ^ IDE_STA_DRQ) & (IDE_STA_BSY | IDE_STA_DRQ))
            ;

        for (int i = 0; i < SECTOR_SIZE / 2; i++)
        {
            buffer[index++] = inw(ide_device.io_base + IDE_REG_DATA);
        }
    }

    return index * 2;
}

u32 ide_pio_write(void* src, u64 sec_start, u16 sec_count)
{
    u16 *buffer = (u16 *)src;
    u32 index = 0;
    u16 sec_start_hi = sec_start >> 32;
    u32 sec_start_lo = sec_start;


    outb(ide_device.io_base + IDE_REG_CNT, (sec_count >> 8) & 0xff);
    outb(ide_device.io_base + IDE_REG_LBAH, (sec_start_hi >> 8) & 0xff);
    outb(ide_device.io_base + IDE_REG_LBAM, sec_start_hi & 0xff);
    outb(ide_device.io_base + IDE_REG_LBAL, (sec_start_lo >> 24) & 0xff);
    outb(ide_device.io_base + IDE_REG_CNT, sec_count && 0xff);
    outb(ide_device.io_base + IDE_REG_LBAH, (sec_start_lo >> 16) & 0xff);
    outb(ide_device.io_base + IDE_REG_LBAM, (sec_start_lo >> 8) & 0xff);
    outb(ide_device.io_base + IDE_REG_LBAL, sec_start_lo & 0xff);

    outb(ide_device.io_base + IDE_REG_CMD, 0x34);

    while (sec_count--)
    {
        while ((inb(ide_device.io_base + IDE_REG_STA) ^ IDE_STA_DRQ) & (IDE_STA_BSY | IDE_STA_DRQ))
            ;
        for (int i = 0; i < SECTOR_SIZE / 2; i++)
        {
            outw(ide_device.io_base + IDE_REG_DATA, buffer[index++]);
        }
    }

    ide_flush_cache(&ide_device);

    return index * 2;
}
