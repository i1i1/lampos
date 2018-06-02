#ifndef _TIP_H_
#define _TIP_H_

#include "defs.h"

/* DO NOT CALL! */
void tip_irq();

void tip_init();

void tip_sleep(int ms);

size_t tip_get_ticks();

#endif /* _TIP_H_ */

