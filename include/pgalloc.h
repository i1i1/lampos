#ifndef  _PGALLOC_H_
#define  _PGALLOC_H_

#include "defs.h"
#include "mb_parce.h"

#define PG_PRESENT		1
#define PG_RW			(1 << 1)
#define PG_USER			(1 << 2)
#define PG_ACCESS		(1 << 5)
#define PG_DIRTY		(1 << 6)
#define PG_ALLOCATED		(1 << 9)


void pgfault(size_t error, size_t addr);

/* Virtual page allocator */
void *pgmalloc();
void pgfree(void *pg);
void pginfo();

size_t pgdirflags(void *virt);
size_t pgflags(void *virt);

/* Mapper of pages */
void pgmap(size_t phys, void *virt, size_t flags);

void mem_init(struct mm_area **mmap, int mmap_len);


#endif /* _PHYSPGALLOC_H_ */

