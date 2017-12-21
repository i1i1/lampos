#ifndef _COM_H_
#define _COM_H_

#include "defs.h"


void com_putc(uint16_t port, int s);
void com_puts(uint16_t port, char *s);
int com_getc(uint16_t port);
void com_init(uint16_t port);


#endif /* _COM_H_ */

