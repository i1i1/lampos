#ifndef _DEFS_H_
#define _DEFS_H_

#define KERNEL_BASE		0xC0000000

#define PGDIR			(KERNEL_BASE + 0x400000)

/* VGA text buffer memory mapped address. */
#define VGA_BUFFER_ADDR		0xB8000

/* BIOS Data Area */
#define	BDA_BASE_PORT_ADDRESS	0x0463
#define BDA_NUM_ROWS		0x0484
#define BDA_NUM_ROWS_LEN	1
#define BDA_NUM_COLUMNS		0x044A
#define BDA_NUM_COLUMNS_LEN	2

#define COM1_PORT_ADDRESS	0x3f8
#define COM2_PORT_ADDRESS	0x2f8
#define COM3_PORT_ADDRESS	0x3e8
#define COM4_PORT_ADDRESS	0x2e8

#define KERNEL_CODE		0x8
#define KERNEL_DATA		0x10
#define USER_CODE		0x18
#define USER_DATA		0x20

#ifndef __ASSEMBLER__
	#include <stdint.h>
	typedef uint32_t size_t;
	#ifndef	NULL
		#define	NULL		(void *)0
	#endif

	#define GET_BIT(n, off)		(((n) >> (off)) & 1)
	#define SET_BIT(n, off)		do {					\
						(n) = (n) |  (1 << (off));	\
					} while(0);

	#define CLR_BIT(n, off)		do {					\
						(n) = (n) & ~(1 << (off));	\
					} while(0);

	#define SWAP(a, b)		do {					\
						typeof(a) c;			\
						c = a;				\
						a = b;				\
						b = c;				\
					} while(0)

#endif


#endif	/* _DEFS_H_ */

