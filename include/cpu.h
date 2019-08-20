#ifndef _CPU_H_
#define _CPU_H_

#include "defs.h"

#define GDT_PRIVL0	0x0
#define GDT_PRIVL1	BIT(0)
#define GDT_PRIVL2	BIT(1)
#define GDT_PRIVL3	(GDT_PRIVL1|GDT_PRIVL2)
#define GDT_EXEC_FLAG	BIT(2)
#define GDT_DC_FLAG	BIT(3)
#define GDT_RW_FLAG	BIT(4)
#define GDT_GRAN_PAGE	BIT(5)
#define GDT_MODE32	BIT(6)
#define GDT_PRESENT	BIT(7)

#define GDT_USER	(GDT_PRESENT | GDT_PRIVL3 | GDT_MODE32)
#define GDT_KERNEL	(GDT_PRESENT | GDT_PRIVL0 | GDT_MODE32)
#define GDT_CODE	(GDT_EXEC_FLAG | GDT_DC_FLAG | GDT_RW_FLAG)
#define GDT_DATA	(GDT_RW_FLAG)

struct gdtr {
	uint16_t	size;
	uint32_t	offset;
} __attribute__ ((packed));


struct gdt_entry {
	uint32_t	limit_low : 16;
	uint32_t	base_low : 24;
	uint8_t		access_byte : 8;
	uint32_t	limit_high : 4;
	uint8_t		flags : 4;
	uint32_t	base_high : 8;
} __attribute__ ((packed));


#endif	/* _CPU_H_ */

