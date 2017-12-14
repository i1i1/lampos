#ifndef _BUDDYALLOC_H_
#define _BUDDYALLOC_H_

#include "defs.h"

/* Maximum power of buddy */
#define MAXBUDDY		30

/* Flags for areas */
#define	BUDDY_FREE		0
#define	BUDDY_ALLOCATED		1
#define	BUDDY_REALLOCATED	2


struct buddy_lst {
	struct buddy_lst *prev;
	struct buddy_lst *next;
	uint8_t flag;
} __attribute__((packed));


void *balloc(size_t size);
//void *brealloc(void *ptr, size_t size);
void bfree(void *ptr);
void balloc_info();
void balloc_init(int numpages);


#endif /* _BUDDYALLOC_H_ */

