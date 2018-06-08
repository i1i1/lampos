#include "kernel.h"

/* This helper reads a byte from I/O port. */
inline uint8_t
inb(uint16_t port)
{
	uint8_t c;

	__asm__ __volatile__ ("inb %%dx,%%al" : "=a"(c) : "d"(port));

	return c;
}

/* This helper reads a (2-byte) word from I/O port. */
inline uint16_t
inw(uint16_t port)
{
	uint16_t c;

	__asm__ __volatile__ ("inw %%dx,%%ax" : "=a"(c) : "d"(port));

	return c;
}
/* This helper writes a (4-byte) long word to I/O port. */
inline uint32_t
inl(uint16_t port)
{
	uint32_t c;

	__asm__ __volatile__ ("inl %%dx,%%eax" : "=a"(c) : "d"(port));

	return c;
}


/* This helper writes a byte to I/O port. */
inline void
outb(uint16_t port, uint8_t val)
{
	__asm__ __volatile__ ("outb %b0,%w1;" :: "a"(val), "d"(port));
}

/* This helper writes a (2-byte) word to I/O port. */
inline void
outw(uint16_t port, uint16_t val)
{
	__asm__ __volatile__ ("outw %%ax,%%dx" :: "d"(port), "a"(val));
}

/* This helper writes a (4-byte) long word to I/O port. */
inline void
outl(uint16_t port, uint32_t val)
{
	__asm__ __volatile__ ("outl %%eax,%%dx" :: "d"(port), "a"(val));
}

