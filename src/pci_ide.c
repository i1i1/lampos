#include "defs.h"
#include "kernel.h"

#include "pgalloc.h"
#include "pci_ide.h"
#include "pci.h"

enum {
    ATA_DATA   = 0,
    ATA_ERROR      = 1,
    ATA_FEATURES   = 1,
    ATA_SECT_CNT   = 2,
    ATA_LBA_LOW    = 3,
    ATA_LBA_MID    = 4,
    ATA_LBA_HIGH   = 5,
    ATA_DRIVE_HEAD = 6,
    ATA_STATUS     = 7,
    ATA_CMD        = 7,
};

enum {
    ATA_CTRL_DEVICE_CTRL = 0,
    ATA_CTRL_DRIVE_ADDR  = 1,
};

enum {
    ATA_STATUS_ERR_BIT = 0,
    ATA_STATUS_DRQ_BIT = 3,
    ATA_STATUS_BSY_BIT = 7,
};



static struct pci_dev_lst *pci_dev = NULL;
static struct pci_ide ide;
static struct ata ata;


static void
ata_wait_ready(struct ata *ata)
{
    while (inb(ata->port+ATA_STATUS) & BIT(ATA_STATUS_BSY_BIT))
        /* nothing */;
}


static void
ata_set_addr_lba48(struct ata *ata, uint64_t lba)
{
    uint8_t lbas[] = {
        lba >>  0,
        lba >>  8,
        lba >> 16,
        lba >> 24,
        lba >> 32,
        lba >> 40,
    };

    outb(ata->port+ATA_SECT_CNT, 0);
    outb(ata->port+ATA_LBA_LOW,  lbas[3]);
    outb(ata->port+ATA_LBA_MID,  lbas[4]);
    outb(ata->port+ATA_LBA_HIGH, lbas[5]);
    outb(ata->port+ATA_SECT_CNT, 0);
    outb(ata->port+ATA_LBA_LOW,  lbas[0]);
    outb(ata->port+ATA_LBA_MID,  lbas[1]);
    outb(ata->port+ATA_LBA_HIGH, lbas[2]);
}

void
ata_write(struct ata *ata, uint64_t addr, uint8_t *buf, int buflen)
{
    uint16_t *b = (void *)buf;

    ata_wait_ready(ata);

    outb(ata->port+ATA_DRIVE_HEAD, 0x40);

    ata_set_addr_lba48(ata, addr/512);

    /* WRITE SECTORS EXT */
    outb(ata->port+ATA_CMD, 0x34);

    ata_wait_ready(ata);

    outsl(ata->port+ATA_DATA, b, sizeof(b)/sizeof(uint32_t));
}

/*
 * Tryes to read bytes enough to fill the buffer.
 */
int
ata_read(struct ata *ata, uint64_t addr, uint8_t *buf, int buflen)
{
    uint16_t *b = (void *)buf;

    ata_wait_ready(ata);

    outb(ata->port+ATA_DRIVE_HEAD, 0x40);

    ata_wait_ready(ata);

    ata_set_addr_lba48(ata, addr/512);

    ata_wait_ready(ata);

    /* READ SECTORS EXT */
    outb(ata->port+ATA_CMD, 0x24);

    ata_wait_ready(ata);

    insl(ata->port+ATA_DATA, b, sizeof(b)/sizeof(uint32_t));

    return 0;
}

/*
 * Returns 1 if drive is accessable.
 */
static int
ata_identify_primary(struct ata *ata, struct pci_ide *ide)
{
    uint16_t ident[256];
    uint8_t status, lbamid, lbahigh;
    int i;

    /* Sending identify to primary port */
    outb(ide->primary_port+ATA_DRIVE_HEAD, 0xA0);

    /* Setting sector count and LBA to 0 */
    outb(ide->primary_port+ATA_SECT_CNT, 0x00);
    outb(ide->primary_port+ATA_LBA_LOW,  0x00);
    outb(ide->primary_port+ATA_LBA_MID,  0x00);
    outb(ide->primary_port+ATA_LBA_HIGH, 0x00);

    /* Sending identify command */
    outb(ide->primary_port+ATA_CMD, 0xEC);

    do {
        status = inb(ide->primary_port+ATA_STATUS);
        if (status == 0) /* drive doesn't exist */
            return 0;
    } while (status & BIT(ATA_STATUS_BSY_BIT));

    lbamid  = inb(ide->primary_port+ATA_LBA_MID);
    lbahigh = inb(ide->primary_port+ATA_LBA_HIGH);

    /* this is not ata driver */
    if (lbamid != 0 && lbahigh != 0)
        return 0;

    do {
        status = inb(ide->primary_port+ATA_STATUS);
        /* if error occured */
        if (status & BIT(ATA_STATUS_ERR_BIT))
            return 0;
    } while (!(status & BIT(ATA_STATUS_DRQ_BIT))); /* should be set when it is ready */

    for (i = 0; i < ARRAY_SIZE(ident); i++)
        ident[i] = inw(ide->primary_port+ATA_DATA);

    *ata = (struct ata) {
        .port       = ide->primary_port,
        .ctl_port   = ide->primary_ctl_port,
        .lba48      = (ident[83]) ? TRUE : FALSE,
        .lbasectors = *((uint32_t *)&ident[60]),
        .sectors    = *((uint64_t *)&ident[100]),
    };

    iprintf("\n");
    iprintf("\tlba48        %s\n", ata->lba48 ? "TRUE" : "FALSE");
    iprintf("\tlbasectors   %x\n", ata->lbasectors);
    iprintf("\tsectors      %lx\n", ata->sectors);

    return 1;
}

static void
ata_init(struct ata *ata, struct pci_ide *ide)
{
    int ret = ata_identify_primary(ata, ide);

    if (ret == 0)
        iprintf("No ata device\n");
    else
        iprintf("Ata device is set!\n");
}

void
cmd_ata_write(uint64_t addr)
{
    uint16_t buf[256];
    int i;

    for (i = 0; i < 256; i++)
        buf[i] = i;

    buf[0] = 0xffff;

    ata_write(&ata, addr, (void *)buf, sizeof(buf));
}

void
cmd_ata_read(uint64_t addr)
{
    uint16_t buf[256];
    int i, j;

    ata_read(&ata, addr, (void *)buf, sizeof(buf));

    iprintf("\nContent of first sector:\n");

    for (i = 0; i < 32; i++) {
        iprintf("\t");
        for (j = 0; j < 8; j++)
            iprintf("%04x ", buf[i*8+j]);
        iprintf("\n");
    }
}

void
pci_ide_init()
{
    uint32_t bar0, bar1, bar2, bar3, bar4;

	pci_dev = pci_get_dev(0x1, 0x1);
	assert_or_panic(pci_dev, "No pci ide dev!");

    bar0 = pci_dev->u._00.BAR0;
    bar1 = pci_dev->u._00.BAR1;
    bar2 = pci_dev->u._00.BAR2;
    bar3 = pci_dev->u._00.BAR3;
    bar4 = pci_dev->u._00.BAR4;

    ide = (struct pci_ide) {
        .primary_port       = (bar0 == 0 || bar0 == 1) ? 0x1F0 : bar0,
        .primary_ctl_port   = (bar1 == 0 || bar1 == 1) ? 0x3F6 : bar1,
        .secondary_port     = (bar2 == 0 || bar2 == 1) ? 0x1F0 : bar2,
        .secondary_ctl_port = (bar3 == 0 || bar3 == 1) ? 0x3F6 : bar3,
        .master_ports       = bar4,
    };

    iprintf("primary ports:   %x %x\n", ide.primary_port,
                                        ide.primary_ctl_port);
    iprintf("secondary ports: %x %x\n", ide.secondary_port,
                                        ide.secondary_ctl_port);
    iprintf("master ports:    %x\n",    ide.master_ports);
    iprintf("program_if:      %x\n",    pci_dev->st.prog_if);

    ata_init(&ata, &ide);
    cmd_ata_write(0x400);
    cmd_ata_read(0x1000);
}

