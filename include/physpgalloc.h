#ifndef _PHYSPGALLOC_H_
#define _PHYSPGALLOC_H_

#include "defs.h"
#include "mb_parce.h"

typedef size_t paddr_t;


paddr_t physpgmalloc();
void physpgfree(paddr_t phys);

void physpginfo();

void physpginit(struct mm_area **mmap, int mmap_len);

#endif /* _PHYSPGALLOC_H_ */

