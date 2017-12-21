#ifndef _MBPARCE_H_
#define _MBPARCE_H_

#include <defs.h>


struct mb_info {
	/* flags indicate validations of next fields */
	uint32_t flags;

	/* lower and upper memory in KB */
	uint32_t mem_lower;
	uint32_t mem_upper;

	/* field that says from what device you've booted */
	uint8_t *boot_device;

	/* command line, C like string */
	char *cmdline;

	/* field which contains information about modifications */
	uint32_t mods_count;
	uint32_t *mods_addr;

	/* ELF SYMBOLS */
	union {
		struct {
			uint32_t tabsize;
			uint32_t strsize;
			uint32_t *addr;

		} a;
		struct {
			uint32_t num;
			uint32_t size;
			uint32_t *addr;
			uint32_t shndx;

		} b;
	} symbols;

	/* field which contains memory map */
	uint32_t mmap_length;
	void *mmap_addr;

	/* drives info */
	uint32_t drives_length;
	uint32_t *drives_addr;

	/* ROM configuration table address */
	uint8_t *config_table;

	/* name of bootloader, C like string */
	char *bootloader;
};

struct mb_mmap {
	uint32_t size;
	uint64_t base_addr;
	uint64_t length;
	uint32_t type;

};

struct mb_header {
	uint32_t magic;
	uint32_t flags;
	uint32_t checksum;

	/* If flag[16] is set */
	uint32_t header_addr;
	uint32_t load_addr;
	uint32_t load_end_addr;
	uint32_t bss_end_addr;
	uint32_t entry_addr;

	/* If flag[2] is set */
	uint32_t mode_type;
	uint32_t width;
	uint32_t height;
	uint32_t depth;

};

struct mm_area {
	uint64_t beg;
	uint64_t end;

};


void mb_parse(struct mb_info *info, struct mm_area ***mm, int *mmlen);


#endif

