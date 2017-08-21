#include <stdint.h>

#include "defs.h"
#include "kernel.h"
#include "physpgalloc.h"
#include "sort.h"

#include "mb_parce.h"


#define MB_MEM_FLAG	(1 << 0)
#define MB_BD_FLAG	(1 << 1)
#define MB_CMD_FLAG	(1 << 2)
#define MB_MODS_FLAG	(1 << 3)
#define MB_SYM_1_FLAG	(1 << 4)
#define MB_SYM_2_FLAG	(1 << 5)
#define MB_MMAP_FLAG	(1 << 6)
#define MB_DRIVES_FLAG	(1 << 7)
#define MB_CT_FLAG	(1 << 8)
#define MB_BOOT_FLAG	(1 << 9)
#define MB_APM_FLAG	(1 << 10)
#define MB_VBE_FLAG	(1 << 11)

#define BUFSZ		4

#define print64(num)		do {					\
					iprintf("%08x", (num) >> 32);	\
					iprintf("%08x", (num));		\
									\
				} while (0)


void area_parse(struct mb_mmap *area, size_t mmap_length,
				struct area **buf, unsigned int *bufsize);

void
mb_parse(struct mb_info *info)
{
	unsigned int i;

	iprintf("\n\t");

	for (i = 11; i != 0; i--)
		iprintf("%d", GET_BIT(info->flags, i));

	iprintf("\n");

	if (info->flags & MB_MEM_FLAG) {
		iprintf("\tlower bound memory %dK\n", info->mem_lower);
		iprintf("\tupper bound memory %dK\n", info->mem_upper);

	}
	if (info->flags & MB_BD_FLAG) {
		if (*info->boot_device)
			iprintf("\tYou booted from hard drive\n");
		else
			iprintf("\tYou booted from floppy\n");

		for (i = 1; i < 4; i++)
			if (*(info->boot_device + i) == 0xff)
				iprintf("\tUnused partitioning %d\n", i);
			else
				iprintf("\tUsed partitioning %d\n", i);

	}
	if (info->flags & MB_CMD_FLAG)
		iprintf("\tcommand line: \"%s\"\n", info->cmdline);
	if (info->flags & MB_MODS_FLAG) {
		iprintf("\t%d modifications loaded\n", info->mods_count);
		iprintf("\tFirst modification address 0x%x\n", info->mods_addr);

	}
	if (info->flags & MB_SYM_1_FLAG) {
		iprintf("\tsymbols 1 tabsize %d\n", info->symbols.a.tabsize);
		iprintf("\tsymbols 1 strsize %d\n", info->symbols.a.strsize);
		iprintf("\tsymbols 1 address 0x%x\n", info->symbols.a.addr);

	}
	else if (info->flags & MB_SYM_2_FLAG) {
		iprintf("\tsymbols 2 num %d\n", info->symbols.b.num);
		iprintf("\tsymbols 2 size %d\n", info->symbols.b.size);
		iprintf("\tsymbols 2 addr %d\n", info->symbols.b.addr);
		iprintf("\tsymbols 2 shndx %d\n", info->symbols.b.shndx);

	}
	if (info->flags & MB_MMAP_FLAG) {
		struct area arr[BUFSZ], *buf[BUFSZ];
		unsigned int buflen;

		for (i = 0; i < BUFSZ; i++)
			buf[i] = arr + i;

		buflen = BUFSZ;

		area_parse(info->mmap_addr, info->mmap_length, buf, &buflen);
		physpginit(buf, buflen);

	}
	if (info->flags & MB_BOOT_FLAG)
		iprintf("\tbootloader name:\"%s\"\n", info->bootloader);
	/*
	if (info->flags & MB__FLAG) {
	}*/
}

int
area_cmp(const struct area **a, const struct area **b)
{
	if ((*a)->beg < (*b)->beg)
		return -1;
	if ((*a)->beg > (*b)->beg)
		return 1;
	return 0;
}

void
area_parse(struct mb_mmap *area, size_t mmap_length,
				struct area **buf, unsigned int *buflen)
{
	unsigned int i, j, prev;

	j = 0;

	while (mmap_length >= 24 && j < *buflen) {
		if (area->type == 1) {
			buf[j]->beg = area->base_addr;
			buf[j++]->end = area->base_addr + area->length;

		}

		mmap_length -= area->size;
		area = (struct mb_mmap *)((unsigned char *)area + area->size + 4);
		mmap_length -= 4;

	}
/*
	//TEST
	j = 6;

	buf[0]->beg = 3, buf[0]->end = 5;
	buf[1]->beg = 0, buf[1]->end = 2;
	buf[2]->beg = 1, buf[2]->end = 3;
	buf[3]->beg = 0, buf[3]->end = 2;
	buf[4]->beg = 10, buf[4]->end = 15;
	buf[5]->beg = 11, buf[5]->end = 12;
*/
	*buflen = j;

	sort(buf, *buflen, sizeof(void *), area_cmp);

	for (i = 0; i < *buflen; i++) {
		iprintf("\tbeg = 0x");
		print64(buf[i]->beg);
		iprintf("\tend = 0x");
		print64(buf[i]->end);
		iprintf("\n");

	}

	iprintf("\n");

	for (prev = 0, i = 1; i < *buflen;) {
		if (buf[i]->beg <= buf[prev]->end) {
			*buflen -= 1;

			if (buf[i]->end > buf[prev]->end)
				buf[prev]->end = buf[i]->end;

			for (j = i; j < *buflen; j++)
				SWAP(buf[j], buf[j + 1]);

		}
		else prev++, i++;

	}

	for (i = 0; i < *buflen; i++) {
		iprintf("\tbeg = 0x");
		print64(buf[i]->beg);
		iprintf("\tend = 0x");
		print64(buf[i]->end);
		iprintf("\n");

	}
}

