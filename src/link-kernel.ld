OUTPUT_FORMAT(elf32-i386)
ENTRY(start)


PHDRS
{
	headers PT_PHDR ;
	text PT_LOAD ;
	data PT_LOAD ;
	rodata PT_LOAD ;
	bss PT_LOAD ;
}

SECTIONS
{
	. = 0xC0100000;

	.text ALIGN(4K) : AT(ADDR(.text) - 0xC0000000)
	{
		*(.multiboot)
		*(.text)
	} :text :headers

	etext = .;

	.rodata ALIGN(4K) : AT(ADDR(.rodata) - 0xC0000000)
	{
		*(.rodata)
	} :rodata

	.data ALIGN(4K) : AT(ADDR(.data) - 0xC0000000)
	{
		*(.data)
	} :data

	edata = .;

	.bss ALIGN(4K) : AT(ADDR(.bss) - 0xC0000000)
	{
		*(COMMON)
		*(.bss)
	} :bss

	end = .;
}

