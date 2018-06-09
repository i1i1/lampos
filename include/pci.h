#ifndef _PCI_H_
#define _PCI_H_

#include <stdint.h>

/* If one of char * are NULL then next indexes are invalid */
struct pci_linux {
	char *vendor_name;
	uint16_t vendor;

	char *dev_name;
	uint16_t dev;

	char *subsys_name;
	uint16_t subvendor;
	uint16_t subdev;
};

extern int pci_db_n;
extern struct pci_linux pci_db[];

void pci_init(void);

#endif /* _PCI_H_ */
