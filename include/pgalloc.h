#ifndef   _PHYSPGALLOC_H_
#define   _PHYSPGALLOC_H_

#include "defs.h"
#include "mb_parce.h"


/* Helper for translating pages from virtual to physical */
size_t virttophys(void *pg);

/* Virtual page allocator */
void *pgalloc();
void pgfree(void *pg);
void pginfo();

/* Mapper of pages */
void pgmap(size_t phys, void *virt, size_t flags);

void pginit(struct mm_area **mmap, int mmap_len);


#endif /* _PHYSPGALLOC_H_ */

