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

#define panic(args...)			do {					\
						iprintf("\n\nKernel panic:\n\t");\
						iprintf(args);			\
						iprintf("\n");			\
						for (;;)			\
							asm("hlt");		\
					} while (0)

#define assert_or_panic(cond, args...)	do {					\
						if (!(cond))			\
							panic(args);		\
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
inline uint8_t inb(uint16_t port);
inline uint16_t inw(uint16_t port);
inline uint32_t inl(uint16_t port);
inline void outb(uint16_t port, uint8_t val);
inline void outw(uint16_t port, uint16_t val);
inline void outl(uint16_t port, uint32_t val);


#endif /* _KERNEL_H_ */

