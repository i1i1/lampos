#ifndef _BUDDYALLOC_H_
#define _BUDDYALLOC_H_

#include "defs.h"

/* Maximum power of buddy */
#define MAXBUDDY	20


struct buddy_lst {
	struct buddy_lst *next;
	uint8_t flag;
} __attribute__((packed));


void *balloc(size_t size);
void bfree(void *ptr);
void balloc_info();
void balloc_init();


#endif /* _BUDDYALLOC_H_ */

