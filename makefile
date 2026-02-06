# Makefile for Watch-OS
# Build kernel, allocator, and bootable ISO

# Compiler and tools
CC      = gcc
ASM     = nasm
LD      = ld
GRUBMK  = grub-mkrescue
QEMU    = qemu-system-x86_64

# Flags
CFLAGS  = -m64 -ffreestanding -O0 -Wall -Wextra -I./kernel -mno-red-zone -fno-pic
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
             $(BUILD)/vga.o $(BUILD)/kprint.o $(BUILD)/panic.o \
             $(BUILD)/idt.o $(BUILD)/idt_load.o $(BUILD)/exceptions.o \
             $(BUILD)/exceptions_handler.o $(BUILD)/pic.o $(BUILD)/keyboard.o \
             $(BUILD)/irq.o $(BUILD)/timer.o $(BUILD)/pmm.o $(BUILD)/paging.o \
             $(BUILD)/heap.o
ISO_FILE   = watch-os.iso

# Default target
all: $(ISO_FILE)

# Compile assembly entry
$(BUILD)/entry.o: $(SRC)/entry.asm | $(BUILD)
	$(ASM) $(ASMFLAGS) $< -o $@

# Compile kernel C code
$(BUILD)/kernel.o: $(SRC)/kernel.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile allocator
$(BUILD)/allocator.o: $(SRC)/allocator.c $(SRC)/allocator.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile VGA driver
$(BUILD)/vga.o: $(SRC)/vga.c $(SRC)/vga.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile kernel print
$(BUILD)/kprint.o: $(SRC)/kprint.c $(SRC)/kprint.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile panic handler
$(BUILD)/panic.o: $(SRC)/panic.c $(SRC)/panic.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile IDT setup
$(BUILD)/idt.o: $(SRC)/idt.c $(SRC)/idt.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile IDT loader
$(BUILD)/idt_load.o: $(SRC)/idt_load.asm | $(BUILD)
	$(ASM) $(ASMFLAGS) $< -o $@

# Compile exceptions assembly
$(BUILD)/exceptions.o: $(SRC)/exceptions.asm | $(BUILD)
	$(ASM) $(ASMFLAGS) $< -o $@

# Compile exception handler C
$(BUILD)/exceptions_handler.o: $(SRC)/exceptions.c $(SRC)/exceptions.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile PIC driver
$(BUILD)/pic.o: $(SRC)/pic.c $(SRC)/pic.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile keyboard driver
$(BUILD)/keyboard.o: $(SRC)/keyboard.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile IRQ handlers
$(BUILD)/irq.o: $(SRC)/irq.asm | $(BUILD)
	$(ASM) $(ASMFLAGS) $< -o $@

# Compile timer
$(BUILD)/timer.o: $(SRC)/timer.c $(SRC)/timer.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile PMM
$(BUILD)/pmm.o: $(SRC)/pmm.c $(SRC)/pmm.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile paging
$(BUILD)/paging.o: $(SRC)/paging.c $(SRC)/paging.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile heap
$(BUILD)/heap.o: $(SRC)/heap.c $(SRC)/heap.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# Link kernel
$(KERNEL_BIN): $(OBJS) | $(BUILD)
	$(LD) $(LDFLAGS) $(OBJS) -o $(KERNEL_BIN)

# Create bootable ISO
$(ISO_FILE): $(KERNEL_BIN)
	mkdir -p $(BOOT)/grub
	cp $(KERNEL_BIN) $(BOOT)/
	cp boot/grub.cfg $(BOOT)/grub/
	$(GRUBMK) -o $(ISO_FILE) $(ISO)

# Create build directory
$(BUILD):
	mkdir -p $(BUILD)

# Clean
clean:
	rm -rf $(BUILD) $(ISO_FILE) $(ISO)

# Run in QEMU
run: $(ISO_FILE)
	$(QEMU) -cdrom $(ISO_FILE)

# Debug mode
debug: $(ISO_FILE)
	$(QEMU) -cdrom $(ISO_FILE) -d int,cpu_reset -no-reboot -no-shutdown

.PHONY: all clean run debug
