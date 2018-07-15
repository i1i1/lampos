#include "kernel.h"
#include "buddyalloc.h"
#include "pit.h"

#include "pci.h"


enum pci_ports {
	CFG_ADDR = 0xCF8,
	CFG_DATA = 0xCFC,
};

static struct pci_dev_lst *head = NULL;
int dev_cnt = 0;


uint32_t
pci_inl(struct pci_dev_lst *d, uint8_t off)
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
	addr.a.bus = d->bus;
	addr.a.slot = d->dev;
	addr.a.func = d->func;
	addr.a.off = off >> 2;

	outl(CFG_ADDR, addr.n);

	return inl(CFG_DATA);
}

struct pci_dev_lst *
pci_get_dev(uint8_t class, uint8_t subclass)
{
	struct pci_dev_lst *dp;

	for (dp = head; dp != NULL; dp = dp->next) {
		if (dp->ref == 0 && dp->st.class == class
		    && dp->st.subclass == subclass) {
			dp->ref++;
			return dp;
		}
	}

	return NULL;
}

void
pci_unget_dev(struct pci_dev_lst *dp)
{
	struct pci_dev_lst *np;

	for (np = head; np != NULL; dp = dp->next) {
		if (np == dp) {
			dp->ref = 0;
			return;
		}
	}

	return;
}

static const struct pci_ext_db *
pci_ext_db_lookup(struct pci_dev_lst *d)
{
	int i, h = d->st.header % 128;
	struct pci_ext_db *cs;

	for (i = 0, cs = pci_db; i < pci_db_n; i++, cs++) {
		switch (h) {
		case 0:
			if (cs->vendor != d->st.vendor)
				continue;
			if (cs->dev != d->st.dev && cs->dev_name != NULL)
				continue;
			if (cs->dev != d->st.dev && cs->dev_name != NULL)
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
pci_dev_scan()
{
	int i, mfunc, h;
	struct pci_dev_lst tmp, *np;
	uint32_t *sp, size;

#define xmalloc(a, n)	do {				\
				void *p = balloc(n);	\
				if (p == NULL) return;	\
				a = p;			\
			} while (0);

	np = head;

	for (tmp.bus = 0; tmp.bus < 256; tmp.bus++) {
		for (tmp.dev = 0; tmp.dev < 32; tmp.dev++) {
			tmp.func = 0;

			/* If (vendor == 0xFFFF) then continue */
			if ((uint16_t)pci_inl(&tmp, 0) == 0xFFFF)
				continue;

			h = pci_inl(&tmp, 0x0C) >> 16;
			mfunc = (h >> 7) ? 8 : 1;

			for (tmp.func = 0; tmp.func < mfunc; tmp.func++) {
				/* If (vendor == 0xFFFF) then continue */
				if ((uint16_t)pci_inl(&tmp, 0) == 0xFFFF)
					continue;

				h = (pci_inl(&tmp, 0x0C) >> 16) % 128;

				if (!head) {
					xmalloc(head, sizeof(tmp));
					np = head;
				} else {
					xmalloc(np->next, sizeof(tmp));
					np = np->next;
				}

				np->bus = tmp.bus;
				np->dev = tmp.dev;
				np->func = tmp.func;
				np->ref = 0;
				np->next = NULL;

				dev_cnt++;

				sp = (void *)&(np->st);

				if (h == 0)
					size = sizeof(np->st) + sizeof(np->u._00);
				else if (h == 1)
					size = sizeof(np->st) + sizeof(np->u._01);
				else if (h == 2)
					size = sizeof(np->st) + sizeof(np->u._02);
				else
					panic("Unknown type of pci device!");

				for (i = 0; i < size; i += 4)
					*sp++ = pci_inl(np, i);
			}
		}
	}
#undef xmalloc
}

void
pci_print_info()
{
	struct pci_dev_lst *np;
	const struct pci_ext_db *info;

	for (np = head; np != NULL; np = np->next) {
		dprintf("\tpci_bus %d, pci_dev %d, func %d\n"
			"\theader %02xh, class %xh, subclass %xh, prog_if %xh\n",
			np->bus, np->dev, np->func, np->st.header % 128,
			np->st.class, np->st.subclass, np->st.prog_if);

		/* Indexes devices via http://pci-ids.ucw.cz/v2.2/pci.ids
		 * Data updates often
		 */
		info = pci_ext_db_lookup(np);
		if (!info) {
			dprintf("\tWOW! Unknown device %04x from vendor %04x!\n",
				np->st.dev, np->st.vendor);
		}
		else {
			dprintf("\tvendor %s, device %s\n",
				info->vendor_name, info->dev_name ? info->dev_name : "(null)");
			if (info->subsys_name)
				dprintf("\tsub_sys %s\n", info->subsys_name);
		}
		dprintf("\n");
	}
	dprintf("TOTAL: %d devices\n", dev_cnt);
}

void
pci_init(void)
{
	pci_dev_scan();
	pci_print_info();
}

