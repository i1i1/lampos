#ifndef _KERNEL_H_
#define _KERNEL_H_

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

#define MAX(a, b)	((a) > (b) ? (a) : (b))
#define MIN(a, b)	((a) < (b) ? (a) : (b))

#define STREQ(a, b)  (strcmp(a, b) == 0)
#define STRNEQ(a, b) (strcmp(a, b) != 0)

#define CONFIG_SET(macro) _is_set_(macro)
#define _macrotest_1 ,
#define _is_set_(value) _is_set__(_macrotest_##value)
#define _is_set__(comma) _is_set___(comma 1, 0)
#define _is_set___(_, v, ...) v

#define BIT(n) (1ul << (n))
#define MASK(n) (BIT(n)-1ul)
#define MASK_BITS(f, t) (MASK(f-t) << t)
#define IS_ALIGNED(n, b) (!((n) & MASK(b)))
#define ROUND_DOWN(n, b) (((n) >> (b)) << (b))
#define ROUND_UP(n, b) (((((n) - 1ul) >> (b)) + 1ul) << (b))
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#define PASTE(a, b) a ## b
#define _STRINGIFY(a) #a
#define STRINGIFY(a) _STRINGIFY(a)


#ifndef __ASSEMBLER__

#include <stdint.h>

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

#endif /* __ASSEMBLER__ */

#endif /* _KERNEL_H_ */

