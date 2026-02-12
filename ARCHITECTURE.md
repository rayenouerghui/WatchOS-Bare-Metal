# WatchOS Architecture - Complete Phase Breakdown

This document explains every phase of WatchOS development, every file created, and why each component exists.

---

## 📋 Table of Contents

1. [Phase 0: Environment Setup](#phase-0-environment-setup)
2. [Phase 1: Boot Process](#phase-1-boot-process)
3. [Phase 2: Core Services](#phase-2-core-services)
4. [Phase 3: Interrupts & Hardware](#phase-3-interrupts--hardware)
5. [Phase 4: Memory Management](#phase-4-memory-management)
6. [Phase 5: Multitasking](#phase-5-multitasking)
7. [Build System](#build-system)
8. [Complete File Tree](#complete-file-tree)

---

## Phase 0: Environment Setup

**Goal:** Prepare development environment and understand toolchain

### Tools Required:
- **gcc** - C compiler (freestanding mode, no libc)
- **nasm** - Assembly compiler for x86_64
- **ld** - Linker to combine object files
- **grub-mkrescue** - Create bootable ISO
- **qemu-system-x86_64** - Virtual machine for testing

### Why These Tools?
- **gcc -ffreestanding**: Compiles C without standard library (we ARE the OS)
- **nasm -f elf64**: Creates 64-bit ELF object files
- **ld**: Places code at specific memory addresses (1MB)
- **GRUB**: Handles complex boot process (firmware → bootloader → kernel)
- **QEMU**: Test without rebooting real hardware

---

## Phase 1: Boot Process

**Goal:** Get from power-on to C code execution

### Files Created:

#### 1. `boot/grub.cfg`
**Purpose:** GRUB bootloader configuration
```
menuentry "Watch-OS" {
    multiboot2 /boot/kernel.bin
    boot
}
```
**Why it exists:**
- Tells GRUB where to find our kernel
- Uses Multiboot2 protocol (standard boot interface)
- GRUB handles: loading kernel, setting up CPU mode, passing memory info

#### 2. `kernel/entry.asm`
**Purpose:** Assembly entry point (first code that runs)
```asm
section .multiboot    ; Multiboot2 header
section .bss          ; Stack space
section .text         ; Code
_start:               ; Entry point
    cli               ; Disable interrupts
    mov rsp, stack_top ; Set up stack
    call kernel_main   ; Jump to C code
```
**Why it exists:**
- CPU starts in a weird state (no stack, random registers)
- Must set up stack before calling C functions
- Multiboot header tells GRUB "this is a valid kernel"
- Stack grows downward, so we point to the top

**Key Concepts:**
- **Multiboot2 header**: Magic numbers that identify kernel format
- **Stack setup**: C functions need a stack for local variables
- **cli**: Disable interrupts until we're ready to handle them

#### 3. `kernel/linker.ld`
**Purpose:** Linker script (memory layout)
```
ENTRY(_start)
SECTIONS {
    . = 0x100000;        /* Load at 1MB */
    .multiboot : { ... } /* Multiboot header FIRST */
    .text : { ... }      /* Code */
    .data : { ... }      /* Initialized data */
    .bss : { ... }       /* Uninitialized data */
}
```
**Why it exists:**
- CPU executes memory, not files
- Linker decides WHERE each section lives in RAM
- 0x100000 (1MB) is safe - below is reserved for BIOS/firmware
- Multiboot header MUST be first (GRUB looks for it there)

**Key Concepts:**
- **Sections**: Different types of data (.text=code, .data=variables, .bss=zeroed memory)
- **Alignment**: Hardware requires certain addresses (4KB boundaries)
- **Symbols**: `heap_start`, `heap_end` exported for memory management

---

## Phase 2: Core Services

**Goal:** Basic kernel infrastructure (output, logging, error handling)

### Files Created:

#### 1. `kernel/vga.c` + `kernel/vga.h`
**Purpose:** VGA text mode driver (80×25 characters)
```c
#define VGA_BUFFER 0xB8000  /* Hardware address */
void vga_putchar(char c, uint8_t color);
void vga_print(const char* str, uint8_t color);
```
**Why it exists:**
- After boot, BIOS services disappear
- Must talk to hardware directly
- VGA memory-mapped I/O: write to 0xB8000 → appears on screen
- Each character = 2 bytes (ASCII + color attribute)

**Key Concepts:**
- **Memory-mapped I/O**: Hardware accessible via memory addresses
- **Volatile**: Tells compiler "this memory can change unexpectedly"
- **Color attributes**: 4 bits background + 4 bits foreground

#### 2. `kernel/kprint.c` + `kernel/kprint.h`
**Purpose:** Kernel logging system (structured output)
```c
typedef enum { LOG_INFO, LOG_OK, LOG_WARN, LOG_ERROR } log_level_t;
void kprint(log_level_t level, const char* message);
```
**Why it exists:**
- Abstracts VGA details (don't call vga_print everywhere)
- Color-coded messages (green=success, red=error)
- Centralized logging for debugging

**Key Concepts:**
- **Abstraction layers**: Hide implementation details
- **Log levels**: Categorize message importance

#### 3. `kernel/panic.c` + `kernel/panic.h`
**Purpose:** Fatal error handler (kernel crash screen)
```c
void panic(const char* message) __attribute__((noreturn));
```
**Why it exists:**
- Unrecoverable errors need safe handling
- Displays error message, then halts CPU
- `__attribute__((noreturn))`: Tells compiler this never returns

**Key Concepts:**
- **Graceful failure**: Better than undefined behavior
- **cli + hlt**: Disable interrupts, halt CPU safely

#### 4. `kernel/allocator.c` + `kernel/allocator.h`
**Purpose:** Simple bump allocator (Phase 2 version)
```c
void* kmalloc(size_t size);  /* Allocate memory */
```
**Why it exists:**
- Need dynamic memory before full heap is ready
- Bump allocator: just increment pointer (no free)
- Uses heap space defined in linker script

**Key Concepts:**
- **Bump allocator**: Simplest allocator (pointer += size)
- **No free()**: Memory never reclaimed (temporary solution)

---

## Phase 3: Interrupts & Hardware

**Goal:** Handle CPU exceptions and hardware interrupts

### Files Created:

#### 1. `kernel/idt.c` + `kernel/idt.h`
**Purpose:** Interrupt Descriptor Table setup
```c
struct idt_entry {
    uint64_t offset_low, selector, ist, type_attr;
    uint64_t offset_mid, offset_high, zero;
} __attribute__((packed));

void idt_init(void);  /* Set up 256 interrupt handlers */
```
**Why it exists:**
- CPU needs to know WHERE to jump when interrupt occurs
- IDT = array of 256 entries (0-31 exceptions, 32+ hardware IRQs)
- Each entry points to handler function

**Key Concepts:**
- **Interrupt vector**: Number identifying interrupt type
- **Handler address**: Where CPU jumps when interrupt fires
- **Type attributes**: Privilege level, gate type

#### 2. `kernel/idt_load.asm`
**Purpose:** Load IDT into CPU
```asm
idt_load:
    lidt [rdi]  ; Load IDT register
    ret
```
**Why it exists:**
- `lidt` instruction only available in assembly
- Tells CPU where IDT is located

#### 3. `kernel/exceptions.asm` + `kernel/exceptions.c` + `kernel/exceptions.h`
**Purpose:** CPU exception handlers (divide by zero, page fault, etc.)
```asm
isr0:  ; Divide by zero
    push 0      ; Dummy error code
    push 0      ; Interrupt number
    jmp isr_common_stub
```
**Why it exists:**
- CPU exceptions = programming errors (invalid memory, bad instruction)
- Must handle gracefully or CPU triple-faults (reboot)
- Assembly stub → C handler (easier to debug)

**Key Concepts:**
- **Exception vs Interrupt**: Exception=CPU error, Interrupt=hardware event
- **Error code**: Some exceptions push error code, others don't
- **Stack frame**: Save all registers before calling C

#### 4. `kernel/pic.c` + `kernel/pic.h`
**Purpose:** Programmable Interrupt Controller driver
```c
void pic_remap(void);           /* Remap IRQs to avoid conflicts */
void pic_send_eoi(uint8_t irq); /* End of Interrupt signal */
```
**Why it exists:**
- PIC manages hardware interrupts (keyboard, timer, disk)
- Default IRQ mapping conflicts with CPU exceptions
- Must remap: IRQ0-15 → INT 32-47

**Key Concepts:**
- **IRQ**: Hardware interrupt request line
- **Remapping**: Change interrupt vector numbers
- **EOI**: Tell PIC "I handled this interrupt"

#### 5. `kernel/irq.asm`
**Purpose:** Hardware interrupt handlers (IRQ0-15)
```asm
irq0_handler:  ; Timer
    push registers
    call timer_handler
    call pic_send_eoi
    pop registers
    iretq
```
**Why it exists:**
- Hardware interrupts need different handling than exceptions
- Must send EOI to PIC (or interrupts stop)
- Assembly wrapper → C handler

#### 6. `kernel/timer.c` + `kernel/timer.h`
**Purpose:** Programmable Interval Timer driver
```c
void timer_init(uint32_t frequency);  /* Set timer frequency */
void timer_handler(void);             /* Called on every tick */
```
**Why it exists:**
- Timer generates periodic interrupts (e.g., 100 Hz)
- Used for: timekeeping, scheduling, timeouts
- PIT = 8254 chip, base frequency 1.193182 MHz

**Key Concepts:**
- **Frequency divisor**: Base freq / desired freq
- **Timer ticks**: Counter incremented on each interrupt

#### 7. `kernel/keyboard.c`
**Purpose:** PS/2 keyboard driver
```c
void keyboard_handler(void);  /* Process keyboard input */
```
**Why it exists:**
- Keyboard generates IRQ1 when key pressed/released
- Must read scancode from port 0x60
- Convert scancode → ASCII character

**Key Concepts:**
- **Scancode**: Hardware key code (not ASCII)
- **Scancode set 1**: Standard PC keyboard mapping
- **Shift key**: Modifier state tracking

---

## Phase 4: Memory Management

**Goal:** Virtual memory, physical page allocation, dynamic heap

### Files Created:

#### 1. `kernel/pmm.c` + `kernel/pmm.h`
**Purpose:** Physical Memory Manager (page allocator)
```c
void pmm_init(uint64_t mem_size);
uint64_t pmm_alloc_page(void);  /* Allocate 4KB page */
void pmm_free_page(uint64_t addr);
```
**Why it exists:**
- Manages physical RAM at page granularity (4KB)
- Bitmap tracks free/used pages (1 bit per page)
- Foundation for virtual memory

**Key Concepts:**
- **Page**: 4KB block of memory (hardware page size)
- **Bitmap**: Efficient tracking (1 bit per page)
- **Physical address**: Real RAM address

#### 2. `kernel/paging.c` + `kernel/paging.h`
**Purpose:** x86_64 4-level paging implementation
```c
void paging_init(void);
void paging_map_page(uint64_t virt, uint64_t phys, uint64_t flags);
void paging_enable(void);  /* Load CR3 */
```
**Why it exists:**
- Virtual memory = isolation, protection, flexibility
- 4-level paging: PML4 → PDPT → PD → PT
- Each process can have own address space

**Key Concepts:**
- **Virtual address**: What programs see
- **Physical address**: Real RAM location
- **Page tables**: Translation structures (virtual → physical)
- **CR3 register**: Points to root page table (PML4)
- **TLB**: Translation cache (must invalidate on unmap)

**Page Table Hierarchy:**
```
Virtual Address (48 bits)
├─ PML4 index (9 bits) → PML4 entry → PDPT
├─ PDPT index (9 bits) → PDPT entry → PD
├─ PD index   (9 bits) → PD entry   → PT
├─ PT index   (9 bits) → PT entry   → Physical page
└─ Offset     (12 bits) → Byte within page
```

#### 3. `kernel/heap.c` + `kernel/heap.h`
**Purpose:** Dynamic memory allocator (malloc/free)
```c
void heap_init(void);
void* heap_alloc(size_t size);
void heap_free(void* ptr);
```
**Why it exists:**
- Replaces simple bump allocator
- Supports free() (memory reclamation)
- Block coalescing prevents fragmentation

**Key Concepts:**
- **Block header**: Metadata (size, free flag, magic number)
- **Free list**: Linked list of available blocks
- **Coalescing**: Merge adjacent free blocks
- **First-fit**: Find first block large enough
- **Splitting**: Divide large block if too big

**Block Structure:**
```
[Header: magic|size|free|next] [User Data...]
```

---

## Phase 5: Multitasking

**Goal:** Multiple processes running concurrently

### Files Created:

#### 1. `kernel/process.c` + `kernel/process.h`
**Purpose:** Process management (PCB, creation, termination)
```c
typedef struct process {
    uint32_t pid;
    process_state_t state;
    cpu_context_t context;  /* Saved registers */
    uint64_t* stack;
    uint64_t time_slice;
    struct process* next;
} process_t;

process_t* process_create(void (*entry_point)(void), uint64_t stack_size);
```
**Why it exists:**
- Process = unit of execution (code + state)
- PCB (Process Control Block) = process metadata
- Each process has own stack and CPU state

**Key Concepts:**
- **PID**: Process identifier (unique number)
- **Process state**: READY, RUNNING, BLOCKED, TERMINATED
- **CPU context**: All registers (RAX-R15, RIP, RFLAGS, CR3)
- **Stack**: Each process needs own stack (8KB default)

#### 2. `kernel/scheduler.c` + `kernel/scheduler.h`
**Purpose:** Process scheduling (decide which process runs)
```c
void scheduler_init(void);
void scheduler_add(process_t* proc);     /* Add to ready queue */
process_t* scheduler_next(void);         /* Get next process */
void scheduler_switch(void);             /* Perform context switch */
```
**Why it exists:**
- Scheduler decides which process runs when
- Round-robin: each process gets equal time
- Ready queue: circular linked list of runnable processes

**Key Concepts:**
- **Scheduling algorithm**: Round-robin (fair, simple)
- **Time slice**: How long process runs (10 timer ticks)
- **Ready queue**: Processes waiting to run
- **Preemption**: Forcibly switch processes (timer-driven)

**Round-Robin Scheduling:**
```
Ready Queue: [A] → [B] → [C] → [A] (circular)
Timer tick → Switch to next process
```

#### 3. `kernel/context_switch.asm`
**Purpose:** Save/restore CPU state (assembly for speed)
```asm
context_switch:
    ; Save old context (all registers)
    mov [rdi + 0], rax
    mov [rdi + 8], rbx
    ...
    ; Load new context
    mov rax, [rsi + 0]
    mov rbx, [rsi + 8]
    ...
    ret  ; Jump to new process
```
**Why it exists:**
- Context switch = save old state, load new state
- Must save ALL registers (including CR3 for page tables)
- Assembly required for direct register access

**Key Concepts:**
- **Context**: Complete CPU state (registers + flags + page table)
- **Atomicity**: Context switch must be uninterruptible
- **Stack switching**: RSP points to new process stack

**What Gets Saved:**
- General registers: RAX, RBX, RCX, RDX, RSI, RDI, RBP, RSP
- Extended registers: R8-R15
- Instruction pointer: RIP (where to resume)
- Flags: RFLAGS (CPU state flags)
- Page table: CR3 (virtual memory mapping)

---

## Build System

### Files:

#### 1. `makefile`
**Purpose:** Automate compilation and linking
```makefile
CFLAGS = -m64 -ffreestanding -O0 -Wall -Wextra -mno-red-zone -fno-pic
ASMFLAGS = -f elf64
LDFLAGS = -n -T kernel/linker.ld

all: watch-os.iso
```
**Why it exists:**
- Compiles each .c/.asm file to .o (object file)
- Links all .o files into kernel.bin
- Creates bootable ISO with GRUB

**Key Flags:**
- `-m64`: 64-bit code
- `-ffreestanding`: No standard library
- `-mno-red-zone`: Required for interrupt handlers
- `-fno-pic`: No position-independent code
- `-O0`: No optimization (easier debugging)

#### 2. `build.sh`
**Purpose:** Convenience script (clean + build + run)
```bash
make clean
make all
make run
```

#### 3. `boot/boot.asm`
**Purpose:** Legacy bootloader (not used with GRUB)
**Why it exists:** Educational reference (shows what GRUB does for us)

---

## Complete File Tree

```
WatchOS-Bare-Metal/
├── boot/
│   ├── boot.asm              # Legacy bootloader (educational)
│   └── grub.cfg              # GRUB configuration
│
├── kernel/
│   ├── entry.asm             # Assembly entry point (Phase 1)
│   ├── linker.ld             # Linker script (Phase 1)
│   │
│   ├── vga.c / vga.h         # VGA driver (Phase 2)
│   ├── kprint.c / kprint.h   # Kernel logging (Phase 2)
│   ├── panic.c / panic.h     # Error handler (Phase 2)
│   ├── allocator.c / allocator.h  # Simple allocator (Phase 2)
│   │
│   ├── idt.c / idt.h         # IDT setup (Phase 3)
│   ├── idt_load.asm          # IDT loader (Phase 3)
│   ├── exceptions.asm        # Exception stubs (Phase 3)
│   ├── exceptions.c / exceptions.h  # Exception handlers (Phase 3)
│   ├── irq.asm               # IRQ stubs (Phase 3)
│   ├── pic.c / pic.h         # PIC driver (Phase 3)
│   ├── timer.c / timer.h     # Timer driver (Phase 3)
│   ├── keyboard.c            # Keyboard driver (Phase 3)
│   │
│   ├── pmm.c / pmm.h         # Physical memory manager (Phase 4)
│   ├── paging.c / paging.h   # Virtual memory (Phase 4)
│   ├── heap.c / heap.h       # Heap allocator (Phase 4)
│   │
│   ├── process.c / process.h # Process management (Phase 5)
│   ├── scheduler.c / scheduler.h  # Scheduler (Phase 5)
│   ├── context_switch.asm    # Context switching (Phase 5)
│   │
│   └── kernel.c              # Main kernel entry point
│
├── build/                    # Compiled object files (generated)
├── iso/                      # ISO filesystem (generated)
├── makefile                  # Build automation
├── build.sh                  # Build script
├── readme.md                 # Project documentation
└── architecture_phase_*.jpg  # Architecture diagrams

```

---

## Why This Architecture?

### Layered Design:
```
Layer 5: Processes & Scheduling
Layer 4: Memory Management (Virtual + Physical)
Layer 3: Interrupts & Hardware Drivers
Layer 2: Core Services (VGA, Logging, Panic)
Layer 1: Boot & Entry
Layer 0: Hardware
```

Each layer builds on the previous:
- **Can't have processes without memory management**
- **Can't have memory management without interrupts** (page faults)
- **Can't have interrupts without core services** (logging errors)
- **Can't have core services without boot** (getting to C code)

### Separation of Concerns:
- **Boot code** (assembly) separate from **kernel logic** (C)
- **Hardware drivers** separate from **abstractions** (VGA vs kprint)
- **Memory management** separate from **process management**

### Why Assembly + C?
- **Assembly**: Required for CPU-specific operations (interrupts, context switch)
- **C**: Easier to write complex logic (memory management, scheduling)
- **No C++**: Too complex for kernel (exceptions, RTTI, constructors)

---

## Key Concepts Summary

### 1. Boot Process
- Firmware → GRUB → Multiboot → entry.asm → kernel_main()
- Stack setup required before C code
- Linker places code at specific addresses

### 2. Memory Management
- **Physical**: PMM tracks 4KB pages with bitmap
- **Virtual**: 4-level paging translates addresses
- **Heap**: Dynamic allocator with free list

### 3. Interrupts
- **IDT**: Table of 256 interrupt handlers
- **Exceptions**: CPU errors (0-31)
- **IRQs**: Hardware interrupts (32+)
- **PIC**: Routes hardware interrupts to CPU

### 4. Multitasking
- **Process**: Code + state + stack
- **Scheduler**: Decides which process runs
- **Context switch**: Save old state, load new state
- **Preemption**: Timer forces process switch

---

## What Makes This a Real OS?

✅ **Boots independently** (no host OS)
✅ **Manages hardware** (VGA, keyboard, timer)
✅ **Handles interrupts** (exceptions + IRQs)
✅ **Virtual memory** (paging with page tables)
✅ **Dynamic memory** (heap with malloc/free)
✅ **Multitasking** (multiple processes, scheduling)

### What's Missing (Future Work):
- ❌ User mode (Ring 3) - all code runs in kernel mode
- ❌ System calls - no kernel/user boundary
- ❌ File system - no persistent storage
- ❌ Networking - no network stack
- ❌ Device drivers - only basic VGA/keyboard
- ❌ ELF loader - can't load external programs

---

## Learning Outcomes

After building WatchOS, you understand:

1. **How computers boot** (firmware → bootloader → kernel)
2. **How CPUs work** (registers, interrupts, privilege levels)
3. **How memory works** (physical vs virtual, paging)
4. **How OSes multitask** (processes, scheduling, context switching)
5. **How hardware is accessed** (memory-mapped I/O, port I/O)
6. **Why abstractions matter** (layers, separation of concerns)

---

## File Count by Phase

- **Phase 0**: 0 files (environment setup)
- **Phase 1**: 3 files (boot/grub.cfg, entry.asm, linker.ld)
- **Phase 2**: 8 files (vga, kprint, panic, allocator)
- **Phase 3**: 12 files (idt, exceptions, irq, pic, timer, keyboard)
- **Phase 4**: 6 files (pmm, paging, heap)
- **Phase 5**: 6 files (process, scheduler, context_switch)
- **Total**: 35 source files + build system

---

**This is how every operating system on Earth starts.** 🚀

From Linux to Windows to macOS - they all:
1. Boot from firmware
2. Set up memory management
3. Handle interrupts
4. Schedule processes

You've built the foundation. Everything else is "just" adding features on top of this core.
