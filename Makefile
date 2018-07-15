KERNEL_LD = src/link-kernel.ld
STAGE2 = build/stage2_eltorito
GENISOIMAGE = genisoimage
CC = gcc
CFLAGS = -Wall -Iinclude -nostdlib -ffreestanding -m32 -std=gnu90

src = $(wildcard src/*.c) src/pci_db.c
hdr = $(wildcard include/*.h)

obj = $(subst src, build, $(src:.c=.o))
dep = $(subst src, build, $(src:.c=.d))

all: default

debug: CFLAGS += -DDEBUG
debug: make boot.iso test

default: CFLAGS += -Werror
default: make boot.iso

gdb: CFLAGS += -ggdb -DDEBUG
gdb: boot.iso
	qemu-system-i386 -cdrom boot.iso -m 128M -gdb tcp::1234 -S &
	gdb -s boot.bin

test: boot.iso usb
	qemu-system-i386 -cdrom boot.iso -m 128M -usb -usbdevice disk:format=raw:usb -serial file:com1

src/pci_db.c:
	./utils/pci_gener/generdata.sh

boot.bin: src/boot.S $(obj)
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
		-boot-load-size 4 -boot-info-table -quiet -o boot.iso iso/

build/%.o: src/%.c build/%.d
	$(CC) $(CFLAGS) -c -T $(KERNEL_LD) -nostdlib -ffreestanding -o $@ $<

make: $(dep)

build/%.d: src/%.c
	./makedeps.awk -v inc=include -v build=build $< > $@
#	$(CC) $(CFLAGS) -MM $< | sed -e "s/^[^ \t]\+\.o:/build\/&/" > $@

# Creates file for usb plugging for qemu
usb:
	dd if=/dev/zero of=$@ bs=1024 count=64000

clean:
	rm -rf iso/
	rm -rf boot.bin boot.iso
	rm -rf $(obj)

fullclean: clean
	rm -rf usb
	rm -rf src/pci_db.c
	rm -rf $(dep)

.PHONY: fullclean clean debug test make
.SECONDARY: $(dep)

ifneq "$(MAKECMDGOALS)" "clean"
-include $(dep)
endif

