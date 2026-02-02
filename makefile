# Makefile for Watch-OS
# Build kernel, allocator, and bootable ISO

# Compiler and tools
CC      = gcc
ASM     = nasm
LD      = ld
GRUBMK  = grub-mkrescue
QEMU    = qemu-system-x86_64

# Flags
CFLAGS  = -m64 -ffreestanding -O2 -Wall -Wextra -I./kernel
ASMFLAGS = -f elf64
LDFLAGS  = -n -T kernel/linker.ld

# Directories
SRC     = kernel
BUILD   = build
ISO     = iso
BOOT    = $(ISO)/boot

# Files
KERNEL_BIN = $(BUILD)/kernel.bin
OBJS       = $(BUILD)/entry.o $(BUILD)/kernel.o $(BUILD)/allocator.o \
             $(BUILD)/vga.o $(BUILD)/kprint.o $(BUILD)/panic.o
ISO_FILE   = watch-os.iso

# Default target
all: $(ISO_FILE)

# Compile assembly
$(BUILD)/entry.o: $(SRC)/entry.asm | $(BUILD)
	$(ASM) $(ASMFLAGS) $< -o $@

# Compile kernel C code
$(BUILD)/kernel.o: $(SRC)/kernel.c $(SRC)/allocator.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile allocator C code
$(BUILD)/allocator.o: $(SRC)/allocator.c $(SRC)/allocator.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile VGA driver
$(BUILD)/vga.o: $(SRC)/vga.c $(SRC)/vga.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile kernel print system
$(BUILD)/kprint.o: $(SRC)/kprint.c $(SRC)/kprint.h $(SRC)/vga.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile panic handler
$(BUILD)/panic.o: $(SRC)/panic.c $(SRC)/panic.h $(SRC)/vga.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# Link kernel
$(KERNEL_BIN): $(OBJS) | $(BUILD)
	$(LD) $(LDFLAGS) $(OBJS) -o $(KERNEL_BIN)

# Prepare ISO folder and copy files
$(ISO_FILE): $(KERNEL_BIN)
	mkdir -p $(BOOT)/grub
	cp $(KERNEL_BIN) $(BOOT)/
	cp boot/grub.cfg $(BOOT)/grub/
	$(GRUBMK) -o $(ISO_FILE) $(ISO)

# Create build directory if it doesn't exist
$(BUILD):
	mkdir -p $(BUILD)

# Clean build and ISO files
clean:
	rm -rf $(BUILD) $(ISO_FILE) $(ISO)

# Run QEMU
run: $(ISO_FILE)
	$(QEMU) -cdrom $(ISO_FILE)

.PHONY: all clean run


