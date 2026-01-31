# Makefile for Watch OS
# Phase 1.3

# Tools
ASM = nasm
CC  = gcc
LD  = ld
GRUB_MKRESCUE = grub-mkrescue

# Paths
KERNEL_SRC = kernel
BUILD_DIR  = build
ISO_DIR    = iso

# Files
ENTRY = $(KERNEL_SRC)/entry.asm
KERNEL_C = $(KERNEL_SRC)/kernel.c
LINKER = $(KERNEL_SRC)/linker.ld
KERNEL_OBJ = $(BUILD_DIR)/entry.o $(BUILD_DIR)/kernel.o
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
ISO_FILE = watch-os.iso

# Colors for print
RED=\033[0;31m
NC=\033[0m

# Default target
all: iso

# Assemble
$(BUILD_DIR)/entry.o: $(ENTRY)
	@mkdir -p $(BUILD_DIR)
	$(ASM) -f elf64 $< -o $@

# Compile
$(BUILD_DIR)/kernel.o: $(KERNEL_C)
	@mkdir -p $(BUILD_DIR)
	$(CC) -m64 -ffreestanding -c $< -o $@

# Link
$(KERNEL_BIN): $(KERNEL_OBJ) $(LINKER)
	$(LD) -n -T $(LINKER) -o $@ $(KERNEL_OBJ)

# Prepare ISO
iso: $(KERNEL_BIN)
	@mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL_BIN) $(ISO_DIR)/boot/kernel.bin
	cp boot/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	$(GRUB_MKRESCUE) -o $(ISO_FILE) $(ISO_DIR)

# Clean
clean:
	rm -rf $(BUILD_DIR) $(ISO_DIR) $(ISO_FILE)
	@echo "${RED}Cleaned build files${NC}"
