#include "kernel.h"
#include "buddyalloc.h"

#include "pci.h"

enum pci_ports {
	CFG_ADDR = 0xCF8,
	CFG_DATA = 0xCFC,
};


/* Damn big structure! */
struct pci_dev {
	uint16_t vendor;
	uint16_t device;

	uint16_t comand;
	uint16_t status;

	uint8_t revision;
	uint8_t prog_if;
	uint8_t subclass;
	uint8_t class_code;

	uint8_t cache_ln_sz;
	uint8_t latency_timer;
	uint8_t header;
	uint8_t BIST;

	uint32_t BAR0;

	uint32_t BAR1;

	uint32_t BAR2;

	uint32_t BAR3;

	uint32_t BAR4;

	uint32_t BAR5;

	void *cardbus_cis;

	uint16_t sub_vendor;
	uint16_t sub_device;

	void *EROM_base_addr;

	uint8_t cap_p;
	uint32_t res0 : 16;

	uint32_t res1;

	uint8_t int_ln;
	uint8_t int_pin;
	uint8_t min_grant;
	uint8_t max_latency;
} __attribute__ ((packed));

struct pci_dev_lst {
	struct pci_dev_lst *next;
	uint8_t bus;
	uint8_t dev;

	struct pci_dev st;
};

static struct pci_dev_lst *head = NULL;
int dev_cnt = 0;


static uint32_t
cfg_inl(uint8_t bus, uint8_t dev, uint8_t func, uint8_t off)
{
	union pci_addr {
		struct {
			uint8_t zero	: 2;
			uint8_t off	: 6;
			uint8_t func	: 3;
			uint8_t slot	: 5;
			uint8_t bus	: 8;
			uint8_t res	: 7;
			uint8_t fl	: 1;
		} __attribute__((packed)) a;
		uint32_t n;
	};

	union pci_addr addr;

	addr.a.fl = 1;
	addr.a.res = addr.a.zero = 0;
	addr.a.bus = bus;
	addr.a.slot = dev;
	addr.a.func = func;
	addr.a.off = off >> 2;

	outl(CFG_ADDR, addr.n);

	return inl(CFG_DATA);
}

void
pci_init(void)
{
#define xmalloc(a, n)	do {				\
				void *p = balloc(n);	\
				if (p == NULL) return;	\
				a = p;			\
			} while (0);

	int bus, dev, i;
	struct pci_dev_lst tmp, *np;
	size_t *sp;

	xmalloc(head, sizeof(struct pci_dev_lst));
	tmp.next = head;
	np = &tmp;

	dprintf("\n");

	for (bus = 0; bus < 256; bus++) {
		for (dev = 0; dev < 32; dev++) {
			/* If (vendor == 0xFFFF) then continue */
			if (cfg_inl(bus, dev, 0, 0) == 0xFFFF)
				continue;

			np = np->next;
			np->bus = bus;
			np->dev = dev;
			dev_cnt++;

			sp = (void *)&(np->st);

			for (i = 0; i < sizeof(np->st) - sizeof(void *); i += 4)
				*sp++ = cfg_inl(bus, dev, 0, i);

			dprintf("Bus %d, dev %d, device %d, subclass %d\n", np->bus,
				np->dev, np->st.device, np->st.subclass);
			dprintf("header %d, vendor 0x%x\n\n", np->st.header,
				np->st.vendor);

			xmalloc(np->next, sizeof(struct pci_dev_lst));
			np->next->next = NULL;
		}
	}

	dprintf("Total PCI: %d!\n", dev_cnt);

	bfree(np->next);

#undef xmalloc
}

