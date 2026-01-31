# 🧠 Minimal x86_64 Operating System (Learning Project)

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
- Write directly to **VGA text mode** (no libraries, no OS)
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

### What GRUB Actually Does

- Loads `kernel.bin` into RAM
- Switches CPU into **64-bit long mode**
- Sets up:
  - Stack
  - Memory map
  - CPU state
- Jumps to `_start`

Your assembly code assumes the CPU is already prepared.

---

### Why `grub.cfg` Can Be Minimal or Empty

In this project:

- GRUB is used only as a **loader**, not a menu system
- A minimal config is enough:

```cfg
menuentry "My OS" {
    multiboot2 /boot/kernel.bin
    boot
}
```

GRUB is not replaced by `boot.asm` — it **executes before it**.

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

## 🖥️ VGA Text Mode – How Output Works

### Is VGA "a screen"?

No.

VGA is **memory-mapped hardware**.

---

### VGA Memory

```text
Address: 0xB8000
```

- Each character cell = 2 bytes
  - Byte 1: ASCII character
  - Byte 2: Color attribute

Writing to this memory **is writing to the screen**.

---

### Why Multiple Monitors Don’t Matter Here

In text mode:

- VGA maps to **one primary display buffer**
- GPU handles routing the signal to monitors
- The kernel just writes memory

Modern multi-monitor logic exists **far above** this level.

You are bypassing drivers and window systems entirely.

---

## 🧩 Where VGA Lives (Hardware Answer)

Historically:

- VGA was a **separate controller**

Modern systems:

- VGA logic is inside the **GPU**
- Memory is mapped into system address space

It is **not RAM** — it is hardware responding to memory writes.

---

# Phase 2 – Kernel Core (🚧 Planned)

> To be implemented and documented.

- Screen driver abstraction
- Basic memory utilities
- Panic handling

---

# Phase 3 – Interrupts & Exceptions (🚧 Planned)

- IDT setup
- CPU exceptions
- Keyboard interrupt

---

# Phase 4 – Memory Management (🚧 Planned)

- Paging
- Physical memory manager
- Heap

---

# Phase 5 – Advanced Features (🚧 Planned)

- Timer
- Multitasking
- User mode

---

## 🧪 Running the OS

```bash
qemu-system-x86_64 -cdrom build/os.iso
```

---

## 📚 Learning Philosophy

This project intentionally avoids shortcuts.

Every file exists to answer **"why"**, not just **"how"**.

---

## 🧑‍💻 Author

**Rayen Ouer**  
IT Engineering Student – Low-Level Systems & OS Internals

---

🔥 If you understand this project, you understand how *every OS on Earth* starts.

