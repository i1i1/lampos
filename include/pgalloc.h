#ifndef  _PGALLOC_H_
#define  _PGALLOC_H_

#include "defs.h"
#include "mb_parce.h"
#include "physpgalloc.h"


#define PG_PRESENT		BIT(0)
#define PG_RW			BIT(1)
#define PG_USER			BIT(2)
#define PG_ACCESS		BIT(5)
#define PG_DIRTY		BIT(6)
#define PG_ALLOCATED		BIT(9)


/* Various representations of virtual address */
typedef union vaddr {
	/* For indexing with Paging structures */
	struct paging {
		uint16_t fl : 12;
		uint16_t ent : 10;
		uint16_t dir : 10;
	} __attribute__ ((packed)) tbl;

	/* For refering to it as Paging structure */
	union vaddr *dir;

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
void pgmap(paddr_t p, vaddr_t v, uint16_t fl);

void mem_init(struct mm_area **mmap, int mmap_len);


#endif /* _PHYSPGALLOC_H_ */

