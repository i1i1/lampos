#ifndef _DEFS_H_
#define _DEFS_H_

#define KERNEL_BASE		0xC0000000

#define PGDIR			(KERNEL_BASE + 0x7F0000)
#define PGTBL0			(PGDIR + 0x1000)
#define PGTBL1			(PGDIR + 0x2000)
#define PGTEMPO			(PGDIR + 0x3000)

/* VGA text buffer memory mapped address. */
#define VGA_BUFFER_ADDR		(KERNEL_BASE + 0xB8000)

/* BIOS Data Area */
#define	BDA_BASE_PORT_ADDRESS	(KERNEL_BASE + 0x0463)
#define BDA_NUM_ROWS		(KERNEL_BASE + 0x0484)
#define BDA_NUM_ROWS_LEN	1
#define BDA_NUM_COLUMNS		(KERNEL_BASE + 0x044A)
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

	#ifdef	DEBUG
		#define	dprintf(...)	iprintf(__VA_ARGS__)
	#else
		#define	dprintf(...)
	#endif

#endif


#endif	/* _DEFS_H_ */

