ARCH=i386
BUILD_DIR=build
ISO_DIR=$(BUILD_DIR)/iso

NASM=nasm
CC=gcc
LD=ld

CFLAGS=-m32 -ffreestanding -O2 -Wall -Wextra -std=gnu11
LDFLAGS=-m elf_i386 -T linker.ld

KERNEL_OBJS=$(BUILD_DIR)/boot.o $(BUILD_DIR)/kernel.o
KERNEL_BIN=$(BUILD_DIR)/kernel.bin
ISO_IMAGE=$(BUILD_DIR)/sayan-os.iso

.PHONY: all kernel iso run clean

all: iso

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/boot.o: boot.asm | $(BUILD_DIR)
	$(NASM) -f elf32 $< -o $@

$(BUILD_DIR)/kernel.o: kernel.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

kernel: $(KERNEL_BIN)

$(KERNEL_BIN): $(KERNEL_OBJS) linker.ld
	$(LD) $(LDFLAGS) $(KERNEL_OBJS) -o $@

iso: $(ISO_IMAGE)

$(ISO_IMAGE): $(KERNEL_BIN) grub/grub.cfg
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL_BIN) $(ISO_DIR)/boot/kernel.bin
	cp grub/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $@ $(ISO_DIR)

run: iso
	qemu-system-i386 -cdrom $(ISO_IMAGE)

clean:
	rm -rf $(BUILD_DIR)
