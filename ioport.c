#include "kernel.h"

/* This helper reads a byte from I/O port. */
uint8_t
inb(uint16_t port)
{
	uint8_t c;
	__asm__ __volatile__ ("inb %%dx,%%al" : "=a"(c) : "d"(port));
	return c;
}

/* This helper reads a (2-byte) word from I/O port. */
uint16_t
inw(uint16_t port)
{
	uint16_t c;

	__asm__ __volatile__ ("inw %%dx,%%ax" : "=a"(c) : "d"(port));
	return c;
}

/* This helper writes a byte to I/O port. */
void
outb(uint16_t port, uint8_t val)
{
	__asm__ __volatile__ ("outb %b0,%w1;" :: "a"(val), "d"(port));
}

/* This helper writes a (2-byte) word to I/O port. */
void
outw(uint16_t port, uint16_t val)
{
	__asm__ __volatile__ ("outw %%ax,%%dx" :: "d"(port), "a"(val));
}

