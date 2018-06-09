#include "kernel.h"
#include "buddyalloc.h"
#include "pit.h"

#include "pci.h"


enum pci_ports {
	CFG_ADDR = 0xCF8,
	CFG_DATA = 0xCFC,
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
	uint8_t class_code;

	uint8_t cache_ln_sz;
	uint8_t latency_timer;
	uint8_t header;
	uint8_t BIST;

	/* Depends on first 7 bits of header */
	union {
		/* Standart */
		struct pci_00h _00;
		/* Pci to Pci */
		struct pci_01h _01;
		/* CardBus bridge */
		struct pci_02h _02;
	} u;
} __attribute__ ((packed));

struct pci_dev_lst {
	struct pci_dev_lst *next;
	uint8_t bus;
	uint8_t dev;
	uint8_t func;

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

static struct pci_linux *
pci_linux_lookup(struct pci_dev *d)
{
	int i, h = d->header % 128;
	struct pci_linux *cs;

	for (i = 0, cs = pci_db; i < pci_db_n; i++, cs++) {
		switch (h) {
		case 0:
			if (cs->vendor != d->vendor)
				continue;
			if (cs->dev != d->dev && cs->dev_name != NULL)
				continue;
			if (cs->dev != d->dev && cs->dev_name != NULL)
				continue;
			if ((cs->subvendor != d->u._00.subvendor
			     || cs->subdev != d->u._00.subdev) && cs->dev_name != NULL)
				continue;
			return cs;
		case 1:
//			break;
		case 2:
//			break;
		default:
			return NULL;
		}
	}

	return NULL;
}

void
pci_init(void)
{
#define xmalloc(a, n)	do {				\
				void *p = balloc(n);	\
				if (p == NULL) return;	\
				a = p;			\
			} while (0);

	int bus, dev, i, func, mfunc;
	uint16_t h;
	struct pci_dev_lst tmp, *np;
	struct pci_linux *lp;
	uint32_t *sp;

	xmalloc(head, sizeof(struct pci_dev_lst));
	tmp.next = head;
	np = &tmp;

	dprintf("\n\nPCI:\n\n");

	for (bus = 0; bus < 256; bus++) {
		for (dev = 0; dev < 32; dev++) {
			/* If (vendor == 0xFFFF) then continue */
			if ((uint16_t)cfg_inl(bus, dev, 0, 0) == 0xFFFF)
				continue;

			h = cfg_inl(bus, dev, 0, 0x0C) >> 16;
			mfunc = (h >> 7) ? 8 : 1;

			for (func = 0; func < mfunc; func++) {
				/* If (vendor == 0xFFFF) then continue */
				if ((uint16_t)cfg_inl(bus, dev, func, 0) == 0xFFFF)
					continue;

				np = np->next;
				np->bus = bus;
				np->dev = dev;
				np->func = func;
				dev_cnt++;

				sp = (void *)&(np->st);

				for (i = 0; i < sizeof(np->st); i += 4)
					*sp++ = cfg_inl(bus, dev, func, i);

				dprintf("\tpci_bus %d, pci_dev %d, func %d\n"
					"\theader %02xh, class %xh, subclass %xh, prog_if %xh\n",
					np->bus, np->dev, np->func, np->st.header % 128,
					np->st.class_code, np->st.subclass, np->st.prog_if);

				/* Can index device via info from:
				 * http://pci-ids.ucw.cz/v2.2/pci.ids
				 * Data updates every day
				 */
				lp = pci_linux_lookup(&np->st);
				if (!lp) {
					dprintf("\tWOW! Unknown device %04x from vendor %04x!\n\n",
						np->st.dev, np->st.vendor);
				}
				else {
					dprintf("\tvendor %s, device %s\n",
						lp->vendor_name, lp->dev_name ? lp->dev_name : "(null)");
					if (lp->subsys_name)
						dprintf("\tsub_sys %s\n\n", lp->subsys_name);
				}

				xmalloc(np->next, sizeof(struct pci_dev_lst));
				np->next->next = NULL;
			}
		}
	}

	dprintf("Total PCI: %d!\n", dev_cnt);

	bfree(np->next);

#undef xmalloc
}

