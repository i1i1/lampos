#include "kernel.h"
#include "cpu.h"


#define PRIVL_KERNEL	0x0
#define PRIVL_USER	(BIT(1)|BIT(0))

#define SEGM_READ	BIT(0)
#define SEGM_WRITE	BIT(1)
#define SEGM_EXEC	BIT(2)

#define GDT_EXEC_FLAG	BIT(2)
#define GDT_DC_FLAG	BIT(3)
#define GDT_RW_FLAG	BIT(4)
#define GDT_GRAN_PAGE	BIT(5)
#define GDT_MODE32	BIT(6)
#define GDT_PRESENT	BIT(7)

struct tss_struct {
	uint16_t	prev, res0;
	uint32_t	esp0;
	uint16_t	ss0, res1;
	uint32_t	esp1;
	uint16_t	ss1, res2;
	uint32_t	esp2;
	uint16_t	ss2, res3;
	uint32_t	cr3;
	uint32_t	eip;
	uint32_t	eflags;
	uint32_t	eax;
	uint32_t	ecx;
	uint32_t	edx;
        uint32_t	ebx;
	uint32_t	esp;
        uint32_t	ebp;
	uint32_t	esi;
        uint32_t	edi;
	uint16_t	es, res4;
	uint16_t	cs, res5;
	uint16_t	ss, res6;
	uint16_t	ds, res7;
	uint16_t	fs, res8;
	uint16_t	gs, res9;
	uint16_t	ldt, res10;
	uint16_t	res11, iombase;
} __attribute__ ((packed));


#define GDT_PRIVL_MASK		(BIT(1)|BIT(0))

#define GDT_ACCESS_PRESENT	BIT(7)
#define GDT_ACCESS_EXEC		BIT(3)
#define GDT_ACCESS_DC		BIT(2)
#define GDT_ACCESS_RW		BIT(1)

#define GDT_FLAG_INPAGES	BIT(3)
#define GDT_FLAG_MODE32		BIT(2)

static struct gdtr gdt_ptr;
static struct gdt_entry gdt_table[6];
static struct tss_struct tss;

static unsigned char interrupt_stack[4096];

/*
 * This is an assembler function defined elsewhere.
 */
extern void load_gdt(struct gdtr *ptr);

static int
gdt_entry_set(unsigned index,
	      unsigned long base, unsigned long limit,
	      unsigned privl, unsigned flags)
{
	struct gdt_entry *gdtep;

	if (index >= ARRAY_SIZE(gdt_table))
		return -1;

	gdtep = &gdt_table[index];

	/* First entry is a special NULL entry. */
	if (index == 0) {
		gdtep->access_byte = 0;
		gdtep->flags = 0;
		return 0;
	}

	gdtep->flags = GDT_FLAG_MODE32;

	/* Adjust granularity to pages for segments >= 2^16. */
	if (limit > 0xFFFF) {
		gdtep->flags |= GDT_FLAG_INPAGES;
		gdtep->limit_low = (limit >> 12) & 0xFFFF;
		gdtep->limit_high = (limit >> 24) & 0xF;
	} else {
		gdtep->limit_low = limit & 0xFFFF;
		gdtep->limit_high = (limit >> 12) & 0xF;
	}

	/* Set privilege level of the segment. */
	gdtep->access_byte = (privl & GDT_PRIVL_MASK) << 5;
	/* Turn on present bit to mark valid segment. */
	gdtep->access_byte |= GDT_ACCESS_PRESENT | BIT(4);

	/*
	 * Write access is never allowed for code segments. Data segments are
	 * always readable.
	 */
	if (flags & SEGM_EXEC) {
		gdtep->access_byte |= GDT_ACCESS_EXEC;
		if (flags & SEGM_READ)
			gdtep->access_byte |= GDT_ACCESS_RW;
	} else {
		if (flags & SEGM_WRITE)
			gdtep->access_byte |= GDT_ACCESS_RW;
	}

	dprintf("idx: %u access_byte=%02x flags=%02x privl=%02x\n", index,
			gdtep->access_byte, gdtep->flags, privl);

	return 0;
}

void
segm_init()
{
	gdt_ptr.size = sizeof(gdt_table) - 1;
	gdt_ptr.offset = (unsigned long)&gdt_table;

	/* Set up a special NUL-segment. */
	gdt_entry_set(0, 0, 0, 0, 0);

	/* Set up kernel code and data segments. */
	gdt_entry_set(1, 0x0, 0xFFFFFFFF, PRIVL_KERNEL, SEGM_READ|SEGM_EXEC);
	gdt_entry_set(2, 0x0, 0xFFFFFFFF, PRIVL_KERNEL, SEGM_READ|SEGM_WRITE);

	/* Set up user code and data segments. */
	gdt_entry_set(3, 0x0, 0xFFFFFFFF, PRIVL_USER, SEGM_READ|SEGM_EXEC);
	gdt_entry_set(4, 0x0, 0xFFFFFFFF, PRIVL_USER, SEGM_READ|SEGM_WRITE);

	/* Set up TSS segment. */
	memset(&tss, 0, sizeof(tss));
	tss.ss0 = KERNEL_DATA;
	tss.esp0 = (uint32_t)&interrupt_stack;
	tss.cs = KERNEL_CODE|PRIVL_USER;
	tss.ss = tss.es = tss.fs = tss.gs = KERNEL_DATA|PRIVL_USER;

	gdt_entry_set(5, (unsigned long)&tss, (unsigned long)&tss + sizeof(tss), PRIVL_USER, SEGM_READ|SEGM_EXEC);

	load_gdt(&gdt_ptr);
}

