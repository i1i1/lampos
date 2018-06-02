#ifndef _PIT_H_
#define _PIT_H_

#include "defs.h"

/* DO NOT CALL! */
void pit_irq();

void pit_init();

void pit_sleep(int ms);

size_t pit_get_ticks();

#endif /* _PIT_H_ */

