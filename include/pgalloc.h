#ifndef  _PGALLOC_H_
#define  _PGALLOC_H_

#include "defs.h"
#include "mb_parce.h"
#include "physpgalloc.h"


#define PG_PRESENT		1
#define PG_RW			(1 << 1)
#define PG_USER			(1 << 2)
#define PG_ACCESS		(1 << 5)
#define PG_DIRTY		(1 << 6)
#define PG_ALLOCATED		(1 << 9)


typedef union {
	struct {
		uint16_t off : 12;
		uint16_t ent : 10;
		uint16_t dir : 10;
	} __attribute__ ((packed)) tbl;
	size_t num;
	void *ptr;
} __attribute__ ((packed)) vaddr_t;


void pgfault(size_t error, size_t addr);

/* Virtual page allocator */
vaddr_t pgmalloc();
void pgfree(vaddr_t pg);
void pgalloc_info();

vaddr_t pgflags(vaddr_t v);

vaddr_t pgdirinfo(vaddr_t v);
void pginfo();

/* Mapper of pages */
void pgmap(paddr_t p, vaddr_t v, uint16_t flags);

void mem_init(struct mm_area **mmap, int mmap_len);


#endif /* _PHYSPGALLOC_H_ */

