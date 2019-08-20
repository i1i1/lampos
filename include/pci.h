#ifndef _PCI_H_
#define _PCI_H_

#include <stdint.h>

/* If one of char * are NULL then next indexes are invalid */
struct pci_ext_db {
	char *vendor_name;
	uint16_t vendor;

	char *dev_name;
	uint16_t dev;

	char *subsys_name;
	uint16_t subvendor;
	uint16_t subdev;
};

struct pci_00h {
	uint32_t BAR0;

	uint32_t BAR1;

	uint32_t BAR2;

	uint32_t BAR3;

	uint32_t BAR4;

	uint32_t BAR5;

	void *cardbus_cis;

	uint16_t subvendor;
	uint16_t subdev;

	void *EROM_base_addr;

	uint8_t cap_p;
	uint32_t res0 : 24;

	uint32_t res1;

	uint8_t int_ln;
	uint8_t int_pin;
	uint8_t min_grant;
	uint8_t max_latency;
} __attribute__ ((packed));

struct pci_01h {
	uint32_t BAR0;

	uint32_t BAR1;

	uint8_t primary_bus;
	uint8_t second_bus;
	uint8_t subord_bus;
	uint8_t second_latency_timer;

	uint8_t io_base;
	uint8_t io_limit;
	uint16_t second_status;

	uint16_t mem_base;
	uint16_t mem_limit;

	uint16_t premem_base;
	uint16_t premem_limit;

	uint32_t premem_base_up;

	uint32_t premem_limit_up;

	uint16_t io_base_up;
	uint16_t io_limit_up;

	uint8_t cap_p;
	uint32_t res0 : 24;

	uint32_t EROM_base;

	uint8_t int_line;
	uint8_t int_pin;
	uint16_t bridge_cntrl;
} __attribute__ ((packed));

struct pci_02h {
	uint32_t cardbus_sock_base;

	uint8_t off_cap;
	uint8_t res0;
	uint16_t sec_stat;

	uint8_t pci_bus;
	uint8_t card_bus;
	uint8_t subord_bus;
	uint8_t cardbus_lat_timer;

	uint32_t MBA0;

	uint32_t ML0;

	uint32_t MBA1;

	uint32_t ML1;

	uint32_t io_base0;

	uint32_t io_limit0;

	uint32_t io_base1;

	uint32_t io_limit1;

	uint8_t int_line;
	uint8_t int_pin;
	uint16_t bridge_cntrl;

	uint16_t subdev;
	uint16_t subvendor;

	uint32_t PCCLM_base;
} __attribute__ ((packed));

struct pci_dev {
	uint16_t vendor;
	uint16_t dev;

	uint16_t comand;
	uint16_t status;

	uint8_t revision;
	uint8_t prog_if;
	uint8_t subclass;
	uint8_t class;

	uint8_t cache_ln_sz;
	uint8_t latency_timer;
	uint8_t header;
	uint8_t BIST;

} __attribute__ ((packed));

struct pci_dev_lst {
	struct pci_dev_lst *next;

	/* For addressing through pci_inl */
	int bus;
	int dev;
	int func;

	/* Displays if device has been
	 * already given via pci_get_dev
	 */
	uint8_t ref;

	struct pci_dev st;

	/* Depends on first 7 bits of header */
	union {
		/* Standart */
		struct pci_00h _00;
		/* Pci to Pci */
		struct pci_01h _01;
		/* CardBus bridge */
		struct pci_02h _02;
	} u;
};

extern int pci_db_n;
extern struct pci_ext_db pci_db[];

struct pci_dev_lst *pci_get_dev(uint8_t class, uint8_t subclass);
void pci_unget_dev(struct pci_dev_lst *dp);
uint32_t pci_inl(struct pci_dev_lst *d, uint8_t off);
void pci_init(void);
void pci_print_info();

#endif /* _PCI_H_ */
