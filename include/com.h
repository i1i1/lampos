#ifndef _COM_H_
#define _COM_H_

#include "defs.h"


void com_putc(size_t port, int s);
void com_puts(size_t port, char *s);
int com_getc(size_t port);
void com_init(size_t port);


#endif /* _COM_H_ */

