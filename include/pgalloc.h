#ifndef   _PHYSPGALLOC_H_
#define   _PHYSPGALLOC_H_

#include "defs.h"
#include "mb_parce.h"

#define PGDIR_PRESENT		1
#define PGDIR_RW		(1 << 1)
#define PGDIR_USER		(1 << 2)
#define PGDIR_ACCESS		(1 << 5)
#define PGDIR_DIRTY		(1 << 6)
#define PGDIR_ALLOCATED		(1 << 9)


/* Helper for translating pages from virtual to physical */
void pgfault(size_t cr2, size_t error) __attribute__((section("int14")));
size_t virttophys(void *pg);

/* Virtual page allocator */
void *pgalloc();
void pgfree(void *pg);
void pginfo();

/* Mapper of pages */
void pgmap(size_t phys, size_t virt, size_t flags) __attribute__((section("int14")));

void pginit(struct mm_area **mmap, int mmap_len);


#endif /* _PHYSPGALLOC_H_ */

