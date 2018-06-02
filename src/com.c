#include "kernel.h"
#include "defs.h"
#include "com.h"


inline int
com_port_valid(uint16_t port)
{
	return (port == COM1_PORT_ADDRESS) ||
			(port == COM2_PORT_ADDRESS) ||
			(port == COM3_PORT_ADDRESS) ||
			(port == COM4_PORT_ADDRESS);
}

inline int
com_trans_empty(uint16_t port)
{
	return !(inb(port + 5) & 0x20);
}

inline void
com_putc(uint16_t port, int c)
{
	if (!com_port_valid(port))
		return;

	while (com_trans_empty(port))
		;

	outb(port, c);
}

void
com_puts(uint16_t port, char *s)
{
	if (!com_port_valid(port))
		return;

	while (*s != '\0')
		com_putc(port, *s++);
}

inline int
com_fifo_empty(uint16_t port)
{
	return !(inb(port + 5) & 0x1);
}

int
com_getc(uint16_t port)
{
	if (!com_port_valid(port))
		return 0;

	while (com_fifo_empty(port))
		;

	return inb(port);
}

void
com_init(uint16_t port)
{
	if (!com_port_valid(port))
		return;

	// Disable DLAB
	outb(port + 3, 0x00);
	// Disable all interrupts
	outb(port + 1, 0x00);
	// Enable DLAB (set baud rate divisor)
	outb(port + 3, 0x80);
	// Set divisor to 1 (115200 baud)
	outb(port + 0, 0x01);
	outb(port + 1, 0x00);
	// Disable DLAB, 7 bits, no parity, one stop bit
	outb(port + 3, 0x02);
	// Enable FIFO, clear FIFO, with 14 byte threshold
	outb(port + 2, 0xC7);
	// IRQs enabled, RTS and DTR set
	outb(port + 4, 0x0B);
}

