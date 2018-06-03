#include "defs.h"

#define PAGE_ALGIN	1<<0
#define MEMORY_INFO	1<<1
#define HEADER_MAGIC	0x1BADB002
#define HEADER_FLAGS	(PAGE_ALGIN | MEMORY_INFO)
#define CHECKSUM	-(HEADER_MAGIC+HEADER_FLAGS)

#define KERNEL_STACK	4096

.section .bss
stack_down:
.align	16
.space	KERNEL_STACK
stack_top:

.section .multiboot
multiboot:
.long HEADER_MAGIC
.long HEADER_FLAGS
.long CHECKSUM

.section .text

.global cli
.global sti
.global load_gdt
.global load_idt
.global multiboot
.global int0_asm_handler
.global int14_asm_handler

.global _start
.global	start
.set	start,	_start - KERNEL_BASE

.type	_start,	@function
_start:
	/* Creating pgtable for first 4 Mb */
	mov	$(PGTBL0-KERNEL_BASE),	%ecx
	xor	%edi,			%edi

4:
	mov	%edi,	%edx	# index
	shl	$12,	%edx	# multiply by 4096
	or	$3,	%edx
	mov	%edx,	(%ecx, %edi, 4)

	inc	%edi

	cmp	$1024,	%edi
	jb	4b

	/* Creating pgtable for second 4 Mb */
	mov	$(PGTBL1-KERNEL_BASE),	%ecx
	xor	%edi,			%edi

8:
	mov	%edi,		%edx
	shl	$12,		%edx
	add	$0x400003,	%edx
	mov	%edx,		(%ecx,	%edi,	4)

	inc	%edi
	cmp	$1024,	%edi
	jb	8b

	/* Enabling paging */
	mov	$(PGDIR-KERNEL_BASE),	%ecx

	/* Mapping [0m; 8m) to [0m; 8m) */
	movl	$(PGTBL0-KERNEL_BASE+0x3),	(%ecx)
	movl	$(PGTBL1-KERNEL_BASE+0x3),	0x4(%ecx)

	mov	$(KERNEL_BASE >> 22),	%edx

	/* Mapping [0m; 8m) to [3g; 3g8m) */
	movl	$(PGTBL0-KERNEL_BASE+0x3),	(%ecx,	%edx,	4)
	inc	%edx
	movl	$(PGTBL1-KERNEL_BASE+0x3),	(%ecx,	%edx,	4)

	mov	%ecx,		%cr3
	mov	%cr0,		%ecx
	or	$0x80000000,	%ecx
	mov	%ecx,		%cr0

	lea	(higher_half), %ecx
	jmp	*%ecx
higher_half:

	mov	$(PGDIR),	%ecx
	movl	$0,		(%ecx)
	movl	$0,		0x4(%ecx)

	mov	%cr3, %ecx
	mov	%ecx, %cr3

	mov	$stack_top,	%esp

	cli

	/*
	 * Call kernel C entry point function.
	 */

	mov	%cr0,		%eax
	addl	$(KERNEL_BASE),	%ebx
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

/* load_idt -- loads Interrupt Descriptor Table to IDTR. */

.type	load_idt	@function
load_idt:
	movl	0x4(%esp),	%eax
	lidt	(%eax)
	ret

.type	int0_asm_handler	@function
int0_asm_handler:
	mov	%cr2,	%eax
	pushl	%eax
	call	zerodivision
	addl	$4,	%esp

	iret

.type	int14_asm_handler	@function
int14_asm_handler:
	mov	%cr2,	%eax
	popl	%ebx
	pushl	%eax
	pushl	%ebx
	call	pgfault
	addl	$8,	%esp

	iret

.global kbd_asm_handler
.type	kbd_asm_handler	@function
kbd_asm_handler:
	pushl   %fs
	pushl   %es
	pushl   %ds
	pushal
	call	kbd_irq
	popal
	popl	%ds
	popl	%es
	popl	%fs
	iret

.global pit_asm_handler
.type	pit_asm_handler	@function
pit_asm_handler:
	pushl   %fs
	pushl   %es
	pushl   %ds
	pushal
	call	pit_irq
	popal
	popl	%ds
	popl	%es
	popl	%fs
	iret

.global def_asm_handler
.type	def_asm_handler	@function
def_asm_handler:
	pushl   %fs
	pushl   %es
	pushl   %ds
	pushal
	call	def_irq
	popal
	popl	%ds
	popl	%es
	popl	%fs
	iret

