#include <defs.h>

#define PAGE_ALGIN	1<<0
#define MEMORY_INFO	1<<1
#define HEADER_MAGIC	0x1BADB002
#define HEADER_FLAGS	(PAGE_ALGIN | MEMORY_INFO)
#define CHECKSUM	-(HEADER_MAGIC+HEADER_FLAGS)

#define KERNEL_STACK	4096

.section .bss
kernel_stack:
.space KERNEL_STACK

.section .multiboot
.long HEADER_MAGIC
.long HEADER_FLAGS
.long CHECKSUM

.section .text

.global _start
.global cli
.global sti
.global load_gdt
.global pgenable

.type	_start,	@function
_start:
	mov	$kernel_stack,	%esp

	/*
	 * Call kernel C entry point function.
	 */

	mov	%cr0,	%eax
	push	%ebx
	push	%eax

	call	main

	add	$8,	%esp

	/*
	 * Disable interrupts and halt the CPU.
	 */
	cli
	hlt

	/*
	 * Jump in the endless loop to local symbol, if CPU somehow
	 * continued execution after halt.
	 */
.Lloop:
	jmp	.Lloop

.size _start, . - _start

cli:
	pushfl
	cli
	popl	%eax
	ret

sti:
	sti
	ret

/* load_gdt -- loads Global Descriptor Table to GDTR. */

.type	load_gdt	@function
load_gdt:
	movl	0x4(%esp),	%eax
	lgdt	(%eax)
	ljmp	$(KERNEL_CODE),	$(.Lfinish)
.Lfinish:
	mov	$(KERNEL_DATA),	%eax
	mov	%eax,	%ss
	mov	%eax,	%ds
	mov	%eax,	%es
	mov	%eax,	%fs
	mov	%eax,	%gs
	ret

.type	pgenable	@function
pgenable:
	movl    0x4(%esp), %eax
	mov	%eax, %cr3

	mov	%cr0, %eax
	or	$0x80000000, %eax
	mov	%eax, %cr0
	jmp	.Pfinish
.Pfinish:
	ret
