#ifndef _PCI_IDE_H_
#define _PCI_IDE_H_

#include "defs.h"
#include "kernel.h"


struct pci_ide {
    uint16_t primary_port;
    uint16_t primary_ctl_port;
    uint16_t secondary_port;
    uint16_t secondary_ctl_port;
    uint16_t master_ports;
};

struct ata {
    bool_t lba48;
    uint32_t lbasectors; /* number of 28 bit lba sectors */
    uint64_t sectors;    /* number of 48 bit sectors */
};


void pci_ide_init();


#endif /* _PCI_IDE_H_ */

