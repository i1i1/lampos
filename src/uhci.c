#include "defs.h"
#include "kernel.h"

#include "pgalloc.h"
#include "pci.h"
#include "uhci.h"


enum uhci_regs {
	UHCI_USBCMD	= 0x00,
	UHCI_USBSTS	= 0x02,
	UHCI_USBINTR	= 0x04,
	UHCI_FRNUM	= 0x06,
	UHCI_FRBASEADD	= 0x08,
	UHCI_SOFMOD	= 0x0C,
	UHCI_PORTSC1	= 0x10,
	UHCI_PORTSC2	= 0x12,
};


static struct pci_dev_lst *dev = NULL;
static uint16_t io_base = 0;
static void *frame = NULL;
static int frame_num = 0;

void
uhci_init()
{
	vaddr_t tmp;

	dev = pci_get_dev(0xC, 0x3);

	assert_or_panic(dev, "No UHCI dev!");
	assert_or_panic(dev->st.header % 128 == 0x00, "Smth wrong with header!");

	if (dev->st.prog_if != 0x00) {
		dprintf("Not UHCI dev found!");
		return;
	}

	assert_or_panic(dev->u._00.BAR4 % 2 == 1, "No IO in PCI CS!");

	io_base = dev->u._00.BAR4 & ~0b11;

	tmp = pgmalloc();

	if (tmp.ptr == NULL)
		return;

	frame = tmp.ptr;
	frame_num = 0;

	outl(io_base + UHCI_FRBASEADD, tmp.num);

	dprintf("\tframe base addr = %p\n", frame);
	dprintf("\tPORTSC1 = 0x%x\n", inw(io_base + UHCI_PORTSC1));
	dprintf("\tPORTSC2 = 0x%x\n", inw(io_base + UHCI_PORTSC2));
}

