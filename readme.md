# x86_64 Operating System

A **from-scratch educational operating system** built to deeply understand how a computer boots, how a kernel is built and loaded, and how low-level hardware (CPU, memory, VGA) is accessed directly.

This project is **not a production OS**. It is a **learning-focused kernel** designed to answer questions like:

- What really happens between power-on and `kernel_main()`?
- Why do object files, linkers, ISOs, and bootloaders exist?
- How does text appear on the screen without an OS?
- Why do we need GRUB if we already have assembly code?

---

## 🎯 Project Goals

- Understand the **entire boot chain** (Firmware → Bootloader → Kernel)
- Build a **freestanding x86_64 kernel**
- Learn **linking, memory layout, and binary formats**
- Run the system inside **QEMU**

---

## 🧱 Project Structure

```text
.
├── boot/
│   └── boot.asm        # Assembly entry point (low-level setup)
├── kernel/
│   └── kernel.c        # C kernel code (kernel_main)
├── linker.ld           # Linker script (memory layout)
├── grub/
│   └── grub.cfg        # GRUB configuration
├── build/
│   ├── boot.o
│   ├── kernel.o
│   ├── kernel.bin
│   └── os.iso
└── README.md
```

---

# Phase 0 – Environment Setup

This project assumes a **Linux environment** (native Linux or WSL). Commands below are for **Ubuntu / Debian-based systems**.

---

## 1️⃣ Required Tools

### Install build essentials

```bash
sudo apt update
sudo apt install -y build-essential
```

### Install NASM (Assembler)

```bash
sudo apt install -y nasm
```

### Install GRUB utilities

```bash
sudo apt install -y grub-pc-bin grub-common xorriso
```

### Install QEMU (x86_64 emulator)

```bash
sudo apt install -y qemu-system-x86
```

---

## 2️⃣ Toolchain Assumptions

- Architecture: **x86_64**
- Binary format: **ELF → flat binary**
- Compiler: `gcc` (freestanding, no libc)
- Linker: `ld`

No standard library is used.

---

# Phase 1 – Architecture & Boot Process
<p align="center">
  <img src="architecture_phase_1.jpg" width="500">
</p>

> This phase explains **why every component exists**, **the order of execution**, and **how control flows from power-on to the kernel**.

The diagram below represents the **full boot pipeline** used in this project.

---

## 🖥️ High-Level Overview

```text
Power On
  ↓
Firmware (BIOS / UEFI)
  ↓
Bootloader (GRUB)
  ↓
Kernel Binary (kernel.bin)
  ↓
Assembly Entry (_start)
  ↓
C Kernel (kernel_main)
```

Each stage exists because **the CPU cannot jump directly to C code** and **cannot understand filesystems, ISOs, or ELF by itself**.

---

## 📦 Build System (Right Side of Diagram)

### Step 1: Source Code

- `boot.asm` → low-level CPU setup
- `kernel.c` → main kernel logic

The CPU **cannot execute C directly**. Everything must become **machine code**.

---

### Step 2: Compilation & Assembly

```text
boot.asm   → boot.o
kernel.c  → kernel.o
```

- `.o` files are **object files**
- They contain machine code but:
  - Not placed at final memory addresses
  - Not executable alone

This separation allows **modular builds**.

---

### Step 3: Linking (linker.ld)

```text
boot.o + kernel.o → kernel.bin
```

The **linker**:

- Decides **where each section lives in memory**
- Resolves symbols (`_start`, `kernel_main`)
- Produces a **single flat binary**

This step is **mandatory** because CPUs execute memory, not files.

---

## 💿 ISO Creation (Top of Diagram)

```text
os.iso
└── boot/
    ├── grub/
    │   └── grub.cfg
    └── kernel.bin
```

### Why an ISO?

- QEMU boots **firmware**, not raw binaries
- Firmware understands **bootable media** (ISO, disk)
- ISO acts like a virtual CD-ROM

---

## 🧭 GRUB – Why It Exists

### "I already have boot.asm, why GRUB?"

Because:

- Firmware **cannot load arbitrary binaries**
- Firmware **does not know your kernel format**
- Firmware **does not set up protected/long mode safely**

GRUB solves all of this.

---

## 🧠 Kernel Execution Flow

### 1️⃣ CPU jumps to `_start`

- Entry point defined in `linker.ld`
- Written in assembly

Responsibilities:

- Set stack pointer
- Clear registers if needed
- Call `kernel_main`

---

### 2️⃣ `kernel_main()` (C code)

At this point:

- CPU is in 64-bit mode
- Paging is enabled
- Stack is valid

Now you are officially **inside your OS**.

---

# Phase 2 – Kernel Core Services
<p align="center">
  <img src="architecture_phase_2.jpg" width="500">
</p>

This phase transforms the kernel from a "booting program" into a structured, debuggable system by introducing core services required by all real operating systems.

## 🎯 Phase Goals

- Learn kernel modularity and clean abstraction layers
- Implement hardware interaction without BIOS help
- Build dynamic memory management from scratch
- Handle fatal errors gracefully (no undefined behavior)

---

## 🧩 Kernel Subsystems

### 🖥️ VGA Driver (vga.c / vga.h)

**What it does:**
- Writes characters directly to VGA memory (0xB8000)
- Manages cursor position and color attributes
- Initializes the 80×25 text mode

**Why it exists:**
- After boot, BIOS services disappear
- The kernel must talk to hardware directly
- This is the lowest-level output layer, hardware-specific

---

### 📝 Kernel Print System (kprint.c / kprint.h)

**What it does:**
- Provides a clean printing interface to the kernel
- Hides VGA details behind simple functions

**Example:**
```c
kprint("Kernel initialized\n");
kprint("Memory: %d MB available\n", total_memory);
```

**Why it exists:**
- Prevents duplicated VGA code everywhere
- Centralizes output behavior
- Acts as the kernel's logging interface

---

### 💥 Panic System (panic.c / panic.h)

**What it does:**
- Handles unrecoverable kernel errors
- Displays clear error messages
- Safely halts the CPU

**Example:**
```c
if (memory == NULL) {
    panic("Out of memory at line %d", __LINE__);
}
```

**Behavior:**
- Prints error message with optional formatting
- Disables interrupts to prevent further damage
- Halts the system permanently

This is your kernel crash screen

---

### 🧠 Dynamic Memory Allocation (allocator.c / allocator.h)

**What it does:**
- Provides kmalloc() and kfree() equivalents
- Enables dynamic data structures in the kernel
- Uses a simple linear (bump) allocator

**Characteristics:**
- No libc dependencies
- No system calls (we are the kernel!)
- Simple, educational design (not optimized for production)

**Why it exists:**
- Real kernels need dynamic memory
- Static arrays won't scale for complex features
- Memory management is required for interrupts, paging, scheduling

---

## 🔁 Execution Flow

### Normal Boot:
```text
kernel_main()
  ├── vga_init()                    # Setup display hardware
  ├── kprint("Booting kernel...")   # First visible output
  ├── allocator_init()              # Initialize heap memory
  ├── kprint("Heap ready at 0x%x")  # Confirm setup
  └── Continue to main kernel logic
```

### Error Handling:
```text
Fatal error detected
     |
     v
panic("Something went wrong")
     |
     v
+---------------------------+
| 1. Print error message    |
| 2. Disable interrupts     |
| 3. Halt CPU (hlt)         |
+---------------------------+
```
# Phase 3 – Interrupts & Exceptions (✅ Complete)

This phase implements the interrupt handling system, allowing the kernel to respond to CPU exceptions and hardware events.

## 🎯 Phase Goals

- Set up Interrupt Descriptor Table (IDT)
- Handle CPU exceptions gracefully
- Manage hardware interrupts (IRQs)
- Implement device drivers (timer, keyboard)

---

## 🧩 Interrupt System Components

### ⚡ Interrupt Descriptor Table (idt.c / idt.h)

**What it does:**
- Creates a table of 256 interrupt handlers
- Maps interrupt vectors to handler functions
- Configures interrupt gates with proper privilege levels

**Structure:**
```c
struct idt_entry {
    uint16_t offset_low;   // Handler address (bits 0-15)
    uint16_t selector;     // Code segment selector
    uint8_t  ist;          // Interrupt Stack Table
    uint8_t  type_attr;    // Gate type and attributes
    uint16_t offset_mid;   // Handler address (bits 16-31)
    uint32_t offset_high;  // Handler address (bits 32-63)
    uint32_t zero;         // Reserved
} __attribute__((packed));
```

**Why it exists:**
- CPU needs to know WHERE to jump when interrupt occurs
- Different interrupts need different handlers
- Hardware requires specific data structure format

---

### 🚨 CPU Exception Handlers (exceptions.asm / exceptions.c / exceptions.h)

**What it does:**
- Handles CPU exceptions (0-31)
- Displays detailed error information
- Safely halts system on fatal errors

**Exception Types:**
- **Divide by Zero** (INT 0)
- **Debug** (INT 1)
- **Page Fault** (INT 14)
- **General Protection Fault** (INT 13)
- **Invalid Opcode** (INT 6)
- And 27 more...

**Why it exists:**
- CPU exceptions = programming errors
- Must handle gracefully or CPU triple-faults (reboot)
- Provides debugging information (error code, interrupt number)

**Example Output:**
```
*** CPU EXCEPTION ***
Exception: Page Fault
Number: 0x0E
Error Code: 0x0002
System halted.
```

---

### 🎛️ Programmable Interrupt Controller (pic.c / pic.h)

**What it does:**
- Remaps hardware IRQs to avoid conflicts
- Masks/unmasks specific interrupt lines
- Sends End-of-Interrupt (EOI) signals

**PIC Remapping:**
```
Default:  IRQ0-7  → INT 0-7   (conflicts with CPU exceptions!)
          IRQ8-15 → INT 8-15  (conflicts with CPU exceptions!)

Remapped: IRQ0-7  → INT 32-39 (safe)
          IRQ8-15 → INT 40-47 (safe)
```

**Why it exists:**
- Default IRQ mapping conflicts with CPU exceptions
- Must remap to avoid confusion
- PIC routes hardware interrupts to CPU

---

### ⏱️ Timer Driver (timer.c / timer.h)

**What it does:**
- Programs the Programmable Interval Timer (PIT)
- Generates periodic interrupts (100 Hz)
- Maintains system tick counter

**Configuration:**
```c
timer_init(100);  // 100 Hz = 10ms per tick
```

**Why it exists:**
- Provides time-based events
- Enables preemptive multitasking
- Used for timeouts and delays

**How it works:**
- PIT base frequency: 1.193182 MHz
- Divisor = 1193182 / desired_frequency
- Sends IRQ0 on each tick

---

### ⌨️ Keyboard Driver (keyboard.c)

**What it does:**
- Handles keyboard interrupts (IRQ1)
- Converts scancodes to ASCII characters
- Tracks modifier keys (Shift)

**Scancode Translation:**
```
Scancode 0x1E → 'a' (normal)
Scancode 0x1E → 'A' (with Shift)
```

**Why it exists:**
- Keyboard generates hardware interrupts
- Must read scancode from port 0x60
- Provides user input capability

---

## 🔁 Interrupt Flow

### Hardware Interrupt (e.g., Keyboard):
```text
1. User presses key
2. Keyboard sends signal to PIC
3. PIC sends IRQ1 to CPU
4. CPU looks up INT 33 in IDT
5. CPU jumps to irq1_handler (assembly)
6. Assembly saves registers
7. Calls keyboard_handler() (C)
8. C code processes scancode
9. Sends EOI to PIC
10. Assembly restores registers
11. CPU returns to interrupted code (iretq)
```

### CPU Exception (e.g., Page Fault):
```text
1. CPU detects invalid memory access
2. CPU pushes error code to stack
3. CPU looks up INT 14 in IDT
4. CPU jumps to isr14 (assembly)
5. Assembly saves registers
6. Calls exception_handler() (C)
7. C code displays error and halts
```

---

## 📊 Interrupt Vector Map

| Vector | Type | Handler | Description |
|--------|------|---------|-------------|
| 0-31 | Exception | isr0-31 | CPU exceptions |
| 32 | IRQ0 | irq0_handler | Timer (PIT) |
| 33 | IRQ1 | irq1_handler | Keyboard |
| 34-47 | IRQ2-15 | irq2-15_handler | Other hardware |
| 48-255 | Available | - | User-defined |

---

# Phase 4 – Memory Management (✅ Complete)

This phase implements a complete memory management system with physical page allocation, virtual memory, and dynamic heap.

## 🎯 Phase Goals

- Manage physical RAM at page granularity
- Implement x86_64 4-level paging
- Provide dynamic memory allocation (malloc/free)
- Enable memory protection and isolation

---

## 🧩 Memory Management Components

### 📦 Physical Memory Manager (pmm.c / pmm.h)

**What it does:**
- Tracks free/used physical pages (4KB each)
- Allocates and frees physical memory
- Uses bitmap for efficient tracking

**Bitmap Structure:**
```
1 bit per page:
  0 = free
  1 = used

For 32MB RAM:
  8192 pages × 1 bit = 1024 bytes bitmap
```

**API:**
```c
pmm_init(32 * 1024 * 1024);    // Initialize with 32MB
uint64_t page = pmm_alloc_page(); // Allocate 4KB page
pmm_free_page(page);              // Free page
```

**Why it exists:**
- Foundation for virtual memory
- Prevents memory leaks at hardware level
- Efficient tracking (1 bit per 4KB)

---

### 🗺️ Paging System (paging.c / paging.h)

**What it does:**
- Implements x86_64 4-level paging
- Translates virtual addresses to physical addresses
- Manages page tables dynamically

**Page Table Hierarchy:**
```
Virtual Address (48 bits):
┌─────────┬─────────┬─────────┬─────────┬────────────┐
│ PML4 (9)│ PDPT (9)│  PD (9) │  PT (9) │ Offset(12) │
└─────────┴─────────┴─────────┴─────────┴────────────┘
     ↓         ↓         ↓         ↓          ↓
   PML4  →  PDPT  →   PD   →   PT   → Physical Page
```

**Each table has 512 entries (9 bits)**

**API:**
```c
paging_init();                                    // Set up page tables
paging_map_page(virt, phys, PAGE_PRESENT | PAGE_WRITE);
paging_unmap_page(virt);                         // Unmap page
uint64_t phys = paging_get_physical(virt);       // Translate address
paging_enable();                                  // Load CR3
```

**Why it exists:**
- Virtual memory = isolation between processes
- Each process can have own address space
- Memory protection (read-only, user/kernel)
- Enables demand paging (future)

**Page Flags:**
- `PAGE_PRESENT` (bit 0): Page is in memory
- `PAGE_WRITE` (bit 1): Page is writable
- `PAGE_USER` (bit 2): Accessible from user mode

---

### 🏗️ Heap Allocator (heap.c / heap.h)

**What it does:**
- Provides malloc/free functionality
- Manages dynamic memory with block headers
- Coalesces free blocks to prevent fragmentation

**Block Structure:**
```c
struct block_header {
    uint32_t magic;      // 0xDEADBEEF (corruption detection)
    uint32_t size;       // Block size in bytes
    uint8_t is_free;     // 1 = free, 0 = allocated
    struct block_header* next;  // Next block
};
```

**Memory Layout:**
```
Heap Start (0x10000000):
┌──────────────┬─────────────┬──────────────┬─────────────┐
│ Header (16B) │ Data (128B) │ Header (16B) │ Data (256B) │
│ magic|size|  │             │ magic|size|  │             │
│ free|next    │             │ free|next    │             │
└──────────────┴─────────────┴──────────────┴─────────────┘
```

**API:**
```c
heap_init();                    // Initialize heap
void* ptr = heap_alloc(1024);   // Allocate 1KB
heap_free(ptr);                 // Free memory
heap_stats(&total, &used, &free); // Get statistics
```

**Why it exists:**
- Replaces simple bump allocator
- Supports memory reclamation (free)
- Prevents fragmentation with coalescing
- Detects corruption and double-free

**Allocation Strategy:**
1. Search free list for suitable block (first-fit)
2. Split block if too large
3. Mark block as used
4. Return pointer to data area

**Deallocation Strategy:**
1. Mark block as free
2. Coalesce with next block if free
3. Coalesce with previous block if free

---

## 🗺️ Memory Map

```
Physical Memory:
0x000000 - 0x0FFFFF   Kernel code/data (1MB)
0x100000 - 0x10FFFF   Kernel heap (64KB, linker-defined)
0x110000 - 0x11FFFF   PMM bitmap (~4KB for 32MB)
0x120000 - 0x1FFFFFF  Free physical pages

Virtual Memory:
0x000000 - 0x3FFFFF   Identity mapped (kernel, 4MB)
0x10000000+           Heap (256MB virtual address)
```

---

## 🔒 Safety Features

1. **Magic Number Validation**
   - Each heap block has 0xDEADBEEF
   - Detects memory corruption

2. **Double-Free Detection**
   - PMM and heap check for double-free
   - Triggers kernel panic

3. **Bounds Checking**
   - PMM validates page addresses
   - Heap checks for out-of-memory

4. **Memory Alignment**
   - All allocations 16-byte aligned
   - Prevents alignment faults

---

# Phase 5 – Multitasking (✅ Complete)

This phase implements preemptive multitasking with process management and scheduling.

## 🎯 Phase Goals

- Create and manage multiple processes
- Implement round-robin scheduler
- Perform context switching
- Enable preemptive multitasking

---

## 🧩 Multitasking Components

### 🎭 Process Management (process.c / process.h)

**What it does:**
- Creates and manages processes
- Maintains Process Control Block (PCB)
- Tracks process state and resources

**Process Control Block:**
```c
typedef struct process {
    uint32_t pid;              // Process ID
    process_state_t state;     // READY, RUNNING, BLOCKED, TERMINATED
    cpu_context_t context;     // Saved CPU state
    uint64_t* stack;           // Kernel stack
    uint64_t stack_size;       // Stack size (8KB default)
    uint64_t time_slice;       // Remaining time (10 ticks)
    struct process* next;      // Next in queue
} process_t;
```

**CPU Context (saved during context switch):**
```c
typedef struct {
    uint64_t rax, rbx, rcx, rdx;  // General purpose
    uint64_t rsi, rdi, rbp, rsp;  // Pointer registers
    uint64_t r8, r9, r10, r11;    // Extended registers
    uint64_t r12, r13, r14, r15;  // Extended registers
    uint64_t rip;                  // Instruction pointer
    uint64_t rflags;               // CPU flags
    uint64_t cr3;                  // Page table base
} cpu_context_t;
```

**API:**
```c
process_init();                              // Initialize system
process_t* proc = process_create(func, 8192); // Create process
process_exit();                              // Terminate current
process_t* current = process_current();      // Get current process
```

**Why it exists:**
- Process = unit of execution
- PCB stores all process state
- Enables multiple programs to run

---

### 📅 Scheduler (scheduler.c / scheduler.h)

**What it does:**
- Decides which process runs next
- Manages ready queue (circular linked list)
- Implements round-robin algorithm

**Round-Robin Scheduling:**
```
Ready Queue (circular):
┌───┐    ┌───┐    ┌───┐
│ A │ → │ B │ → │ C │ → (back to A)
└───┘    └───┘    └───┘

Time slice expires → Switch to next process
```

**API:**
```c
scheduler_init();              // Initialize scheduler
scheduler_add(proc);           // Add to ready queue
scheduler_remove(proc);        // Remove from queue
process_t* next = scheduler_next(); // Get next process
scheduler_switch();            // Perform context switch
scheduler_yield();             // Voluntarily give up CPU
```

**Why it exists:**
- Fair CPU time distribution
- Prevents process starvation
- Simple and predictable

**Scheduling Algorithm:**
1. Get current process
2. Decrement time slice
3. If time slice = 0 or process blocked:
   - Save current process state
   - Get next process from queue
   - Restore next process state
   - Jump to next process

---

### 🔄 Context Switching (context_switch.asm)

**What it does:**
- Saves current CPU state
- Loads new process CPU state
- Switches page tables (CR3)

**Assembly Implementation:**
```asm
context_switch:
    ; Save old context (RDI points to old_ctx)
    mov [rdi + 0],   rax
    mov [rdi + 8],   rbx
    ; ... save all 15 registers ...
    mov [rdi + 128], rip  ; Return address
    mov [rdi + 136], rflags
    mov [rdi + 144], cr3  ; Page table
    
    ; Load new context (RSI points to new_ctx)
    mov rax,  [rsi + 0]
    mov rbx,  [rsi + 8]
    ; ... load all 15 registers ...
    mov cr3, [rsi + 144]  ; Switch page tables
    
    ret  ; Jump to new process
```

**Why it exists:**
- Must save ALL CPU state
- Assembly required for register access
- Fast and atomic operation

**What Gets Saved:**
- 16 general-purpose registers (RAX-R15)
- Instruction pointer (RIP)
- CPU flags (RFLAGS)
- Page table pointer (CR3)

---

## 🔁 Multitasking Flow

### Process Creation:
```text
1. Allocate PCB (Process Control Block)
2. Allocate stack (8KB)
3. Initialize CPU context:
   - RIP = entry_point (function to run)
   - RSP = stack_top (stack grows down)
   - RFLAGS = 0x202 (interrupts enabled)
4. Add to ready queue
```

### Context Switch (Timer Interrupt):
```text
1. Timer interrupt fires (every 10ms)
2. CPU jumps to irq0_handler
3. Calls timer_handler()
4. timer_handler() calls scheduler_switch()
5. Scheduler:
   - Decrements current process time slice
   - If time slice = 0:
     a. Save current process context
     b. Add current process to ready queue
     c. Get next process from queue
     d. Call context_switch(old_ctx, new_ctx)
6. context_switch():
   - Saves all registers to old_ctx
   - Loads all registers from new_ctx
   - Switches page tables (CR3)
   - Returns to new process
7. New process continues execution
```

### Visual Example:
```
Time: 0ms
Process A running... (time_slice=10)

Time: 10ms - Timer interrupt
  → Save A's state
  → Load B's state
Process B running... (time_slice=10)

Time: 20ms - Timer interrupt
  → Save B's state
  → Load C's state
Process C running... (time_slice=10)

Time: 30ms - Timer interrupt
  → Save C's state
  → Load A's state
Process A running... (time_slice=10)

(Cycle repeats)
```

---

## 🎬 Demo Processes

The kernel creates 3 test processes to demonstrate multitasking:

```c
void process_a(void) {
    while (1) {
        vga_print("A", VGA_COLOR_LIGHT_GREEN);
        // Busy wait
    }
}

void process_b(void) {
    while (1) {
        vga_print("B", VGA_COLOR_LIGHT_CYAN);
        // Busy wait
    }
}

void process_c(void) {
    while (1) {
        vga_print("C", VGA_COLOR_YELLOW);
        // Busy wait
    }
}
```

**Expected Output:**
```
ABCABCABCABCABCABCABCABC...
```

Each letter appears as processes take turns running.

---

## 📊 Process States

```
┌─────────┐
│  READY  │ ←──────────────┐
└────┬────┘                 │
     │                      │
     │ scheduler_next()     │ time slice expired
     ↓                      │
┌─────────┐                 │
│ RUNNING │ ────────────────┘
└────┬────┘
     │
     │ process_exit()
     ↓
┌────────────┐
│ TERMINATED │
└────────────┘
```

---

## 🧪 Running the OS

### Build the OS:
```bash
make clean
make all
```

### Run in QEMU:
```bash
make run
```

### Debug mode (with CPU logs):
```bash
make debug
```

### Expected Output:
```
WatchOS Kernel starting...
Setting up IDT...
✓ IDT initialized (256 entries)
Remapping PIC...
✓ PIC remapped (IRQ0-15 -> INT 32-47)

=== Phase 4: Memory Management ===
Initializing Physical Memory Manager...
✓ PMM initialized
Setting up paging...
✓ Paging initialized (identity mapped 4MB)
✓ Paging enabled (CR3 loaded)
Initializing heap allocator...
✓ Heap allocator initialized (1MB at 0x10000000)

=== Phase 5: Multitasking ===
Initializing process management...
✓ Process management initialized
Initializing scheduler...
✓ Scheduler initialized (round-robin)
Creating test processes...
✓ Created 3 test processes (A, B, C)

Initializing timer (100 Hz)...
✓ Timer initialized and enabled
Enabling keyboard interrupt...
✓ Keyboard IRQ enabled
Enabling interrupts...
✓ Interrupts enabled

✓ WatchOS Kernel booted successfully!
✓ Phase 5 Complete - Multitasking Operational
Watch the A, B, C characters appear (round-robin scheduling)

ABCABCABCABCABCABC... (processes switching)
```

---

## 🏗️ Build System

### Makefile Targets:

- `make all` - Build kernel and create ISO
- `make clean` - Remove build artifacts
- `make run` - Run in QEMU
- `make debug` - Run with debug output

### Compilation Flags:

```makefile
CFLAGS = -m64 -ffreestanding -O0 -Wall -Wextra -mno-red-zone -fno-pic
```

- `-m64`: 64-bit code generation
- `-ffreestanding`: No standard library
- `-O0`: No optimization (easier debugging)
- `-Wall -Wextra`: All warnings enabled
- `-mno-red-zone`: Required for interrupt handlers
- `-fno-pic`: No position-independent code

### Linking:

```makefile
LDFLAGS = -n -T kernel/linker.ld
```

- `-n`: Disable page alignment
- `-T kernel/linker.ld`: Use custom linker script

---

## 📁 Project Structure (Complete)

```text
WatchOS-Bare-Metal/
├── boot/
│   ├── boot.asm              # Legacy bootloader (educational)
│   └── grub.cfg              # GRUB configuration
│
├── kernel/
│   ├── entry.asm             # Assembly entry point
│   ├── linker.ld             # Linker script
│   ├── kernel.c              # Main kernel entry
│   │
│   ├── vga.c / vga.h         # VGA driver
│   ├── kprint.c / kprint.h   # Kernel logging
│   ├── panic.c / panic.h     # Error handler
│   ├── allocator.c / allocator.h  # Simple allocator
│   │
│   ├── idt.c / idt.h         # IDT setup
│   ├── idt_load.asm          # IDT loader
│   ├── exceptions.asm        # Exception stubs
│   ├── exceptions.c / exceptions.h  # Exception handlers
│   ├── irq.asm               # IRQ stubs
│   ├── pic.c / pic.h         # PIC driver
│   ├── timer.c / timer.h     # Timer driver
│   ├── keyboard.c            # Keyboard driver
│   │
│   ├── pmm.c / pmm.h         # Physical memory manager
│   ├── paging.c / paging.h   # Virtual memory
│   ├── heap.c / heap.h       # Heap allocator
│   │
│   ├── process.c / process.h # Process management
│   ├── scheduler.c / scheduler.h  # Scheduler
│   └── context_switch.asm    # Context switching
│
├── build/                    # Compiled objects (generated)
├── iso/                      # ISO filesystem (generated)
│
├── makefile                  # Build automation
├── build.sh                  # Build script
├── readme.md                 # This file
├── ARCHITECTURE.md           # Detailed architecture documentation
├── architecture_phase_1.jpg  # Boot process diagram
└── architecture_phase_2.jpg  # Core services diagram
```

---

## 🎓 What You'll Learn

By building and understanding WatchOS, you'll learn:

### Low-Level Concepts:
- How computers boot (firmware → bootloader → kernel)
- How CPUs work (registers, interrupts, privilege levels)
- How memory works (physical vs virtual, paging)
- How hardware is accessed (memory-mapped I/O, port I/O)

### Operating System Concepts:
- Process management and scheduling
- Virtual memory and page tables
- Interrupt handling and device drivers
- Memory allocation algorithms
- Context switching and multitasking

### Systems Programming:
- Freestanding C (no standard library)
- x86_64 assembly language
- Linker scripts and memory layout
- Build systems and toolchains
- Debugging without an OS

---

## 🚀 Future Enhancements

Potential additions for learning:

### Phase 6 - User Mode:
- Ring 3 execution
- System calls (syscall/sysret)
- User/kernel boundary
- Privilege level transitions

### Phase 7 - Advanced Memory:
- Copy-on-write (COW)
- Demand paging
- Swap support
- Memory-mapped files

### Phase 8 - File System:
- VFS (Virtual File System)
- Simple file system (e.g., FAT)
- File operations (open, read, write, close)
- Directory management

### Phase 9 - IPC:
- Pipes
- Message queues
- Shared memory
- Semaphores and mutexes

### Phase 10 - Networking:
- Network stack (TCP/IP)
- Ethernet driver
- Socket API
- Network protocols

---

## 🐛 Debugging Tips

### QEMU Debug Mode:
```bash
make debug
```
Shows CPU exceptions, register dumps, and interrupt activity.

### Common Issues:

**Triple Fault (Reboot Loop):**
- Check initialization order
- Verify interrupts disabled during setup
- Check page table mappings

**Page Fault:**
- Verify virtual address is mapped
- Check page flags (present, write, user)
- Use `paging_get_physical()` to debug

**General Protection Fault:**
- Check segment selectors
- Verify privilege levels
- Check stack alignment

**Heap Corruption:**
- Look for buffer overflows
- Check for double-free
- Verify magic numbers

### Useful Commands:
```bash
# Check kernel binary format
file build/kernel.bin

# Dump section headers
objdump -h build/kernel.bin

# List symbols
nm build/kernel.bin | grep -E "kernel_main|_start|heap"

# Disassemble code
objdump -d build/kernel.bin | less
```

---

## 📚 Learning Resources

### Recommended Reading:
- **OSDev Wiki**: https://wiki.osdev.org/
- **Intel 64 and IA-32 Architectures Software Developer's Manual**
- **"Operating Systems: Three Easy Pieces"** by Remzi H. Arpaci-Dusseau
- **"Modern Operating Systems"** by Andrew S. Tanenbaum

### Related Projects:
- **Linux Kernel**: https://github.com/torvalds/linux
- **SerenityOS**: https://github.com/SerenityOS/serenity
- **ToaruOS**: https://github.com/klange/toaruos

---

## 📊 Project Statistics

- **Total Lines of Code**: ~3,500
- **Source Files**: 35
- **Assembly Files**: 6
- **C Files**: 24
- **Header Files**: 13
- **Phases Completed**: 5
- **Development Time**: Educational project

---

## 🤝 Contributing

This is an educational project. Feel free to:
- Fork and experiment
- Add new features
- Improve documentation
- Report issues

---

## 📜 License

This project is for educational purposes. Use it to learn, teach, and explore OS development.

---

## 📚 Learning Philosophy

This project intentionally avoids shortcuts.

Every file exists to answer **"why"**, not just **"how"**.

---

## 🧑‍💻 Author

**Mohamed Rayen Ouerghui**  
IT Engineering Student – Low-Level Systems & OS Internals

---

🔥 If you understand this project, you understand how *every OS on Earth* starts.

