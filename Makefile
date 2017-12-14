KERNEL_LD = src/link-kernel.ld
STAGE2 = build/stage2_eltorito
GENISOIMAGE = genisoimage
CC = i586-elf-gcc
CFLAGS = -O3 -Wall -I./include -nostdlib -ffreestanding -m32

src = main.c vga.c libk.c ioport.c printf.c segm.c pgalloc.c sort.c \
	mb_parce.c com.c buddyalloc.c interrupt.c
sources = $(addprefix src/, $(src))
headers = vga.h pgalloc.h sort.h mb_parce.h com.h buddyalloc.h interrupt.h

obj = $(src:.c=.o)
objects = $(addprefix build/object/, $(obj))


all: premake boot.iso

premake:
	@mkdir -p build/object/

test: all
	@qemu-system-i386 -cdrom boot.iso -m 128M &

build/object/%.o: kernel.h defs.h

vga.o: include/vga.h

build/object/%.o: src/%.c
	$(CC) $(CFLAGS) -c -T $(KERNEL_LD) -nostdlib -ffreestanding -o $@ $<

boot.bin: src/boot.S $(objects)
	$(CC) $(CFLAGS) -T $(KERNEL_LD) -o $@ $^ -lgcc

boot.iso: boot.bin
	mkdir -p iso/boot/grub
	cp $(STAGE2) iso/boot/grub/stage2_eltorito
	cp boot.bin iso/boot/boot.bin
	echo "default 0" > iso/boot/grub/menu.lst
	echo "timeout 5" >> iso/boot/grub/menu.lst
	echo "title LAMPOS Dev Starter Kit Kernel" >> iso/boot/grub/menu.lst
	echo "kernel /boot/boot.bin" >> iso/boot/grub/menu.lst
	$(GENISOIMAGE) -R -b boot/grub/stage2_eltorito -no-emul-boot\
		-boot-load-size 4 -boot-info-table -o boot.iso iso/

clean:
	-rm -rf boot.bin boot.iso
	-rm -rf $(objects)

.PHONY: clean premake

