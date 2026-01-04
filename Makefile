#
# Makefile - Raspberry Pi Zero 2 W Bare-Metal Kernel
#
# Requires: aarch64-none-elf toolchain (ARM GNU Toolchain)
# Download from: https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads
#

# Cross-compiler prefix (adjust if using different toolchain)
# CROSS = aarch64-none-elf-
# Using Linux toolchain (from apt install gcc-aarch64-linux-gnu)
CROSS = aarch64-linux-gnu-

CC = $(CROSS)gcc
AS = $(CROSS)gcc
LD = $(CROSS)ld
OBJCOPY = $(CROSS)objcopy
OBJDUMP = $(CROSS)objdump

# Output
BUILD_DIR = build
KERNEL_ELF = $(BUILD_DIR)/kernel8.elf
KERNEL_IMG = $(BUILD_DIR)/kernel8.img

# Compiler flags
CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles
CFLAGS += -mcpu=cortex-a53 -mgeneral-regs-only
CFLAGS += -I./include

# Assembler flags
ASFLAGS = -mcpu=cortex-a53

# Linker flags
LDFLAGS = -nostdlib -T linker.ld

# Source files
ASM_SRCS = src/boot.S
C_SRCS = src/drivers/mailbox.c \
         src/drivers/framebuffer.c \
         src/kernel/sysinfo.c \
         src/kernel/kernel.c \
         src/lib/string.c

# Object files
ASM_OBJS = $(ASM_SRCS:src/%.S=$(BUILD_DIR)/%.o)
C_OBJS = $(C_SRCS:src/%.c=$(BUILD_DIR)/%.o)
OBJS = $(ASM_OBJS) $(C_OBJS)

# Default target
all: dirs $(KERNEL_IMG) boot_files

# Create build directories
dirs:
	@mkdir -p $(BUILD_DIR)/drivers
	@mkdir -p $(BUILD_DIR)/kernel
	@mkdir -p $(BUILD_DIR)/lib

# Compile assembly
$(BUILD_DIR)/%.o: src/%.S
	$(AS) $(ASFLAGS) -c $< -o $@

# Compile C
$(BUILD_DIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Link
$(KERNEL_ELF): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@

# Create binary image
$(KERNEL_IMG): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@
	@echo ""
	@echo "=== Build Complete ==="
	@echo "Kernel image: $(KERNEL_IMG)"
	@ls -lh $(KERNEL_IMG)

# Copy boot files
boot_files: $(KERNEL_IMG)
	@cp boot/config.txt $(BUILD_DIR)/
	@echo ""
	@echo "=== SD Card Files ==="
	@echo "Copy the following to your SD card FAT32 partition:"
	@echo "  1. $(BUILD_DIR)/kernel8.img"
	@echo "  2. $(BUILD_DIR)/config.txt"
	@echo "  3. Raspberry Pi firmware files (bootcode.bin, start.elf, fixup.dat)"
	@echo ""
	@echo "Firmware available at:"
	@echo "  https://github.com/raspberrypi/firmware/tree/master/boot"

# Disassembly (for debugging)
disasm: $(KERNEL_ELF)
	$(OBJDUMP) -d $< > $(BUILD_DIR)/kernel.dis
	@echo "Disassembly: $(BUILD_DIR)/kernel.dis"

# Clean
clean:
	rm -rf $(BUILD_DIR)

# Show size info
size: $(KERNEL_ELF)
	$(CROSS)size $<

# QEMU emulation (Pi 3 closest to Zero 2 W in QEMU)
# Note: QEMU's raspi3b doesn't perfectly match Zero 2 W hardware
qemu: $(KERNEL_IMG)
	qemu-system-aarch64 -M raspi3b -kernel $(KERNEL_IMG) -serial stdio

.PHONY: all dirs boot_files disasm clean size qemu
