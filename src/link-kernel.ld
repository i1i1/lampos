ENTRY(_start)

SECTIONS
{
	. = 1M;

	.text BLOCK(4K): ALIGN(4K)
	{
		*(.multiboot)
		*(.text)
	}

	etext = .;

	.rodata BLOCK(4K): ALIGN(4K)
	{
		*(.rodata)
	}

	.data BLOCK(4K): ALIGN(4K)
	{
		*(.data)
	}

	edata = .;

	.bss BLOCK(4K): ALIGN(4K)
	{
		*(.bss)
	}

	end = .;
	. = 8M;

	int14_s = .;

	int14 BLOCK(4K): ALIGN(4K)
	{
		*(int14)
	}

	int14_e = .;
}

