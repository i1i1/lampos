CC = gcc -m32 -std=gnu90
CFLAGS = -Wall -Iinclude -nostdlib -ffreestanding -fstack-protector-explicit

KERNEL_LD = src/link-kernel.ld
LDFLAGS = -lgcc -T $(KERNEL_LD)

STAGE2 = utils/grub/stage2_eltorito
GENISOIMAGE = genisoimage

SRC := $(shell find src -name "*.c") src/pci_db.c
HDR := $(shell find include -name "*.h")
OBJS := src/boot.S

OUT := boot.bin
ISO := boot.iso

JOBS:=8


#default: CFLAGS += -Werror
default: $(ISO)

debug: CFLAGS += -DDEBUG
debug: $(ISO) test

gdb: CFLAGS += -ggdb -DDEBUG
gdb: $(ISO)
	qemu-system-i386 -cdrom $(ISO) -m 128M -gdb tcp::1234 -S -device isa-debug-exit,iobase=0xf4,iosize=0x04 &
	gdb -s $(OUT)

test: $(ISO) disk
	qemu-system-i386 -cdrom $(ISO) -m 128M -serial file:com1 -device isa-debug-exit,iobase=0xf4,iosize=0x04 -hda disk

src/pci_db.c:
	./utils/pci_gener/generdata.sh

disk:
	qemu-img create disk 4G

$(ISO): $(OUT)
	mkdir -p iso/boot/grub
	cp $(STAGE2) iso/boot/grub/stage2_eltorito
	cp $(OUT) iso/boot/$(OUT)
	echo "default 0" > iso/boot/grub/menu.lst
	echo "timeout 5" >> iso/boot/grub/menu.lst
	echo "title LAMPOS Dev Starter Kit Kernel" >> iso/boot/grub/menu.lst
	echo "kernel /boot/$(OUT)" >> iso/boot/grub/menu.lst
	$(GENISOIMAGE) -R -b boot/grub/stage2_eltorito -no-emul-boot\
		-boot-load-size 4 -boot-info-table -quiet -o $(ISO) iso/

userclean:
	rm -rf iso/
	rm -rf $(BIN) $(ISO)

.PHONY: fullclean default debug test gdb

