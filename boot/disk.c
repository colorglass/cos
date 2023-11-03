#include <type.h>
#include <utils.h>
#include <mem.h>

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

#define SECTOR_SIZE 512

struct ide_device
{
    u8 bus;
    u8 select;
    u16 io_base;
    u16 ctrl_base;
};

static struct ide_device ide_device = {0, 0, IDE0_BASE, IDE0_CTRL};

// /**
//  * host action to make soft reset to device, see ATA/ATAPI-6 spec 9.2
//  * this should apply soft reset to both master and slave devices
// */
// static int ata_soft_reset(struct ide_device* ide_dev)
// {
//     outb(ide_dev->ctrl_base + IDE_REG_CTRL, IDE_CTL_SRST);

//     outb(ide_dev->ctrl_base + IDE_REG_CTRL, 0);

//     /* Here need some timer to break the loop and prompt error messages */
//     while(inb(ide_dev->io_base + IDE_REG_STA) & IDE_STA_BSY);

//     return 0;
// }

static inline void ide_select(struct ide_device *ide_dev)
{
    outb(ide_dev->io_base + IDE_REG_DRI, 0xa0 | ide_dev->select << 4);
}

static inline void ide_disable_irq(struct ide_device *ide_dev)
{
    outb(ide_dev->ctrl_base + IDE_REG_CTRL, IDE_CTL_NIEN);
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

    return VAILD;
}

int ide_init()
{
    if (ide_detect(&ide_device) == INVAILD)
    {
        printf("IDE0 is not vaild or exist, the bootloader only supports IDE0 (hda) as boot disk for now\n");
        return -1;
    }

    ide_disable_irq(&ide_device);

    return 0;
}

u32 ide_pio_read(u32 dest, u64 sec_start, u16 sec_count)
{
    u16 *buffer = (u16 *)dest;
    u32 index = 0;
    u16 sec_start_hi = sec_start >> 32;
    u32 sec_start_lo = sec_start;

    while (inb(ide_device.io_base + IDE_REG_STA) & IDE_STA_BSY)
        ;

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
        while ((inb(ide_device.io_base + IDE_REG_STA) & IDE_STA_DRQ) ^ (IDE_STA_BSY | IDE_STA_DRQ))
            ;

        for (int i = 0; i < SECTOR_SIZE / 2; i++)
        {
            buffer[index++] = inw(ide_device.io_base + IDE_REG_DATA);
        }
    }

    return index * 2;
}

u32 ide_pio_write(u32 src, u32 size, u64 sec_start)
{
    u16 *buffer = (u16 *)src;
    u32 index = 0;
    u16 sec_count = ((size - 1) / SECTOR_SIZE) + 1;
    u16 sec_start_hi = sec_start >> 32;
    u32 sec_start_lo = sec_start;

    while (inb(ide_device.io_base + IDE_REG_STA) & IDE_STA_BSY)
        ;

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
        while ((inb(ide_device.io_base + IDE_REG_STA) & IDE_STA_DRQ) ^ (IDE_STA_BSY | IDE_STA_DRQ))
            ;
        for (int i = 0; i < SECTOR_SIZE / 2; i++)
        {
            buffer[index++] = inw(ide_device.io_base + IDE_REG_DATA);
        }
    }

    return index * 2;
}
