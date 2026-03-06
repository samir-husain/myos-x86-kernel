ARCH=i386
CC=gcc
AS=nasm
LD=ld

CFLAGS=-m32 -ffreestanding -fno-pie -fno-stack-protector -O2 -Wall -Wextra -I src
LDFLAGS=-m elf_i386 -T linker.ld -z noexecstack

BUILD=build
ISO=$(BUILD)/myos.iso

SRCS_C := $(wildcard src/*.c)
SRCS_S := $(wildcard src/*.s)
OBJS := $(patsubst src/%.c,$(BUILD)/%.o,$(SRCS_C)) $(patsubst src/%.s,$(BUILD)/%.o,$(SRCS_S))

all: $(ISO)

$(BUILD):
	mkdir -p $(BUILD)/iso/boot/grub

$(BUILD)/%.o: src/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: src/%.s | $(BUILD)
	$(AS) -f elf32 $< -o $@

$(BUILD)/kernel.elf: $(OBJS) linker.ld
	$(LD) $(LDFLAGS) $(OBJS) -o $@

$(ISO): $(BUILD)/kernel.elf boot/grub/grub.cfg | $(BUILD)
	cp $(BUILD)/kernel.elf $(BUILD)/iso/boot/kernel.elf
	cp boot/grub/grub.cfg $(BUILD)/iso/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO) $(BUILD)/iso >/dev/null 2>&1

run: $(ISO)
	qemu-system-i386 -cdrom $(ISO) -serial stdio

clean:
	rm -rf $(BUILD)

.PHONY: all run clean