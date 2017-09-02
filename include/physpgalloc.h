#ifndef   _PHYSPGALLOC_H_
#define   _PHYSPGALLOC_H_

#include "defs.h"
#include "mb_parce.h"

typedef size_t physaddr_t;
typedef size_t vaddr_t;


size_t physpgalloc();

void physpgfree(size_t page);

void physpginit(struct area **buf, int buflen);


#endif /* _PHYSPGALLOC_H_ */

