# MyOS (32-bit x86)

A tiny 32-bit x86 OS kernel built from scratch (GRUB + C).

## Features (current)
- Boots via GRUB (multiboot)
- VGA text output
- Serial logging (QEMU `-serial stdio`)
- GDT + IDT + ISR stubs
- PIC remap + IRQ0 (PIT timer) + IRQ1 (keyboard)
- Paging enabled (identity map 0–4MB) + page-fault handler
- Physical Memory Manager (PMM) using Multiboot memory map
- Virtual memory mapping helpers
- Simple heap allocator (`kmalloc`)
- Page allocator (`kpage_alloc/kpage_free`)
- Preemptive task switching (round-robin on PIT)

## Build dependencies (Ubuntu/WSL)
```bash
sudo apt update
sudo apt install -y build-essential nasm grub-pc-bin xorriso qemu-system-i386