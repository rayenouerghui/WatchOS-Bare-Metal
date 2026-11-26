# WatchOS Architecture

WatchOS is a freestanding x86_64 operating-system kernel. It boots through GRUB using the Multiboot2 protocol and provides interrupt handling, memory management, process scheduling, VGA output, keyboard input, and a small interactive interface.

## Boot Flow

```text
Power on
  -> BIOS/UEFI firmware
  -> GRUB bootloader
  -> Multiboot2 kernel loaded at 1 MiB
  -> kernel/entry.S
  -> kernel_main()
  -> system initialization
  -> WatchOS menu
```

## Kernel Initialization

`kernel/kernel.c` initializes subsystems in this order:

1. VGA and kernel printing
2. Interrupt Descriptor Table
3. PIC remapping
4. Physical memory manager
5. Paging
6. Heap allocator
7. Process management
8. Scheduler
9. VGA user interface
10. PIT timer and keyboard IRQs
11. Interrupt delivery and idle loop

## Source Layout

| Path | Purpose |
|------|---------|
| `boot/grub.cfg` | GRUB menu entry for the kernel |
| `kernel/entry.S` | Multiboot2 header, 32-bit to 64-bit transition, stack setup |
| `kernel/linker.ld` | Kernel memory layout and entry point |
| `kernel/kernel.c` | Main kernel initialization and idle loop |
| `kernel/vga.c`, `kernel/kprint.c`, `kernel/panic.c` | Display output and fatal error handling |
| `kernel/idt.c`, `exceptions.S`, `exceptions.c` | Interrupt table and CPU exception handling |
| `kernel/pic.c`, `irq.S` | Hardware interrupt routing and IRQ wrappers |
| `kernel/timer.c`, `keyboard.c` | PIT timer and keyboard drivers |
| `kernel/pmm.c`, `paging.c`, `heap.c` | Physical memory, paging, and heap management |
| `kernel/process.c`, `scheduler.c`, `context_switch.S` | Processes, scheduling, and context switching |
| `kernel/ui.c` | Home menu, system time, Snake demo, and system information |

## Memory Model

- The linker places the kernel in low physical memory at `1 MiB`.
- The physical memory manager tracks 4 KiB pages.
- Paging uses x86_64 page-table structures to provide virtual-to-physical mappings.
- The heap allocator manages dynamic kernel allocations and supports allocation splitting and free-block coalescing.

## Interrupt Model

CPU exceptions use interrupt vectors `0-31`. Hardware IRQs are remapped to vectors `32-47` by the PIC driver. The timer uses IRQ0 and the keyboard uses IRQ1.

Assembly interrupt stubs preserve the interrupted CPU state and call C handlers. The appropriate PIC End-of-Interrupt signal is sent after hardware IRQ handling.

## Scheduling Model

The scheduler maintains a round-robin ready queue. The PIT generates timer ticks at 100 Hz, and the scheduler advances process execution according to process time slices and context state.

## Build Model

The Makefile compiles all active C and GNU assembly sources, links `build/kernel.bin` with `kernel/linker.ld`, copies the kernel into an ISO filesystem, and creates `watch-os.iso` with GRUB.

Generated build output is intentionally ignored by Git:

- `build/`
- `iso/`
- `watch-os.iso`
