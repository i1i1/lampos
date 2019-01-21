#ifndef _KERNEL_H_
#define _KERNEL_H_

#include <stdint.h>
#include <stddef.h>

#include "defs.h"

/*
 * Various usefull macroses.
 */
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE (!FALSE)
#endif

#define panic(...)	do {								\
						iprintf("\n\nKernel panic:\n\t");\
						iprintf(__VA_ARGS__);			\
						iprintf("\n");					\
						for (;;)						\
							halt();						\
					} while (0)

#define assert_or_panic(cond, ...)	do {							\
											if (!(cond))			\
												panic(__VA_ARGS__);	\
										} while (0)

#define NELEMS(x)	(sizeof(x)/sizeof(x[0]))

#define MAX(a, b)	((a) > (b) ? (a) : (b))
#define MIN(a, b)	((a) < (b) ? (a) : (b))


/*
 * Enable/disable hardware interrupts.
 */
unsigned int cli();
void sti();

/*
 * Small libc-like kernel library.
 */
int iprintf(const char *fmt, ...);
int isnprintf(char *buf, size_t size, const char *fmt, ...);
size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);
void *memcpy(void *dst, const void *src, size_t len);
void *memset(void *s, int c, size_t n);


/*
 * I/O port routines.
 */
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);
uint32_t inl(uint16_t port);
void outb(uint16_t port, uint8_t val);
void outw(uint16_t port, uint16_t val);
void outl(uint16_t port, uint32_t val);

extern void halt();

#endif /* _KERNEL_H_ */

