# Raspberry Pi Zero 2 W Bare-Metal Kernel

A custom bootloader and kernel that displays device specifications on HDMI output with classic green-on-black terminal aesthetics.

```
╔══════════════════════════════════════════════════════════════╗
║  RASPBERRY PI ZERO 2 W                                       ║
║  Custom Bare-Metal Kernel v1.0                               ║
╚══════════════════════════════════════════════════════════════╝

=== BOARD INFORMATION ===
Model:          Raspberry Pi Zero 2 W
Revision:       0x902120
Serial:         10000000XXXXXXXX
Firmware:       XXXXXXXX

=== PROCESSOR ===
SoC:            BCM2710A1 (Broadcom)
CPU:            Quad-core ARM Cortex-A53
Architecture:   ARMv8-A (64-bit)
ARM Clock:      1000 MHz
Core Clock:     400 MHz

=== MEMORY ===
ARM Memory:     512 MB @ 0x00000000
GPU Memory:     64 MB @ 0x1C000000
SDRAM Clock:    450 MHz

...
```

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    SD Card (FAT32)                          │
│  bootcode.bin → start.elf → kernel8.img (our kernel)       │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    Boot Sequence                             │
│  1. GPU loads bootcode.bin from SD                          │
│  2. bootcode.bin loads start.elf                            │
│  3. start.elf reads config.txt, loads kernel8.img @ 0x80000 │
│  4. ARM cores released from reset                           │
│  5. boot.S parks cores 1-3, core 0 jumps to kernel_main     │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    Kernel Components                         │
│                                                              │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐         │
│  │   Mailbox   │  │ Framebuffer │  │   Sysinfo   │         │
│  │   Driver    │──│   Driver    │──│   Query     │         │
│  └─────────────┘  └─────────────┘  └─────────────┘         │
│         │                │                │                  │
│         ▼                ▼                ▼                  │
│  ┌─────────────────────────────────────────────────┐        │
│  │              VideoCore GPU                       │        │
│  │   (Mailbox Interface @ 0x3F00B880)              │        │
│  └─────────────────────────────────────────────────┘        │
└─────────────────────────────────────────────────────────────┘
```

## Building

### Prerequisites

Install the AArch64 bare-metal cross-compiler:

**Option 1: ARM GNU Toolchain (recommended)**
```bash
# Download from ARM website:
# https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads
# Get: aarch64-none-elf (AArch64 bare-metal target)

# Extract and add to PATH
export PATH=$PATH:/path/to/arm-gnu-toolchain/bin
```

**Option 2: Linux toolchain (may have extra libraries)**
```bash
# Ubuntu/Debian
sudo apt install gcc-aarch64-linux-gnu

# Then edit Makefile, change:
CROSS = aarch64-linux-gnu-
```

### Build

```bash
# Clone/download this project
cd pi-kernel

# Build
make

# Output files in build/:
#   kernel8.img  - The kernel binary
#   config.txt   - Boot configuration
```

### Build Output

```
=== Build Complete ===
Kernel image: build/kernel8.img
-rw-r--r-- 1 user user 8.5K Jan  3 12:00 build/kernel8.img

=== SD Card Files ===
Copy the following to your SD card FAT32 partition:
  1. build/kernel8.img
  2. build/config.txt
  3. Raspberry Pi firmware files (bootcode.bin, start.elf, fixup.dat)
```

## Deployment

### Prepare SD Card

1. **Format SD card** with FAT32 filesystem (the Pi boot partition)

2. **Download firmware files** from Raspberry Pi:
   ```bash
   # From https://github.com/raspberrypi/firmware/tree/master/boot
   # You need:
   #   - bootcode.bin
   #   - start.elf (or start4.elf for Pi 4)
   #   - fixup.dat (or fixup4.dat for Pi 4)
   
   wget https://github.com/raspberrypi/firmware/raw/master/boot/bootcode.bin
   wget https://github.com/raspberrypi/firmware/raw/master/boot/start.elf
   wget https://github.com/raspberrypi/firmware/raw/master/boot/fixup.dat
   ```

3. **Copy files to SD card**:
   ```
   SD Card (FAT32):
   ├── bootcode.bin    (from Raspberry Pi firmware)
   ├── start.elf       (from Raspberry Pi firmware)
   ├── fixup.dat       (from Raspberry Pi firmware)
   ├── config.txt      (from build/)
   └── kernel8.img     (from build/)
   ```

4. **Insert SD card** into Pi Zero 2 W, connect HDMI, power on

### Expected Output

On HDMI display, you should see green text on black background showing:
- Board model and revision
- Serial number
- Processor information (BCM2710A1, Cortex-A53)
- Clock speeds (ARM, Core, SDRAM)
- Memory configuration
- Network info (WiFi, Bluetooth, MAC address)
- Display configuration

## Project Structure

```
pi-kernel/
├── Makefile              # Build system
├── linker.ld             # Memory layout (kernel @ 0x80000)
├── boot/
│   └── config.txt        # Pi boot configuration
├── include/
│   ├── types.h           # Basic type definitions
│   ├── gpio.h            # BCM2710 peripheral addresses
│   ├── mailbox.h         # VideoCore mailbox interface
│   ├── framebuffer.h     # HDMI framebuffer
│   ├── font8x8.h         # 8x8 bitmap font
│   ├── sysinfo.h         # System information query
│   └── string.h          # String utilities
├── src/
│   ├── boot.S            # AArch64 entry point
│   ├── drivers/
│   │   ├── mailbox.c     # Mailbox implementation
│   │   └── framebuffer.c # Framebuffer driver
│   ├── kernel/
│   │   ├── kernel.c      # Main kernel
│   │   └── sysinfo.c     # Hardware info query
│   └── lib/
│       └── string.c      # String functions
└── build/                # Output directory
```

## Technical Details

### Memory Map (BCM2710)

```
0x00000000 - 0x1BFFFFFF  ARM memory (448 MB)
0x1C000000 - 0x1FFFFFFF  VideoCore GPU memory (64 MB)
0x3F000000 - 0x3FFFFFFF  Peripheral registers
0x40000000 -             Local peripherals (ARM control)
```

### Mailbox Protocol

Communication with VideoCore GPU uses the mailbox interface:
- Write buffer address to `MAILBOX_WRITE` (channel in low 4 bits)
- Poll `MAILBOX_STATUS` until not full/empty
- Read response from `MAILBOX_READ`

Property tags allow querying/setting hardware parameters like:
- Board revision and serial
- Memory configuration
- Clock rates
- Framebuffer allocation

### Boot Process

1. **GPU Boot**: VideoCore GPU starts first, loads `bootcode.bin`
2. **Second Stage**: `bootcode.bin` loads `start.elf` 
3. **ARM Release**: `start.elf` reads `config.txt`, loads kernel, releases ARM cores
4. **Kernel Start**: All 4 ARM cores begin at `_start` (0x80000)
5. **Core Parking**: Cores 1-3 enter WFE loop, core 0 continues
6. **BSS Clear**: Core 0 zeros BSS section
7. **C Entry**: Jump to `kernel_main()`

## Debugging

### UART Output

Add UART support for debug output:
```c
// In gpio.h - UART base
#define UART0_BASE (PERIPHERAL_BASE + 0x201000)

// Add uart.c with init and putc functions
```

### QEMU Emulation

```bash
# Pi 3 is closest to Zero 2 W in QEMU
make qemu

# Note: Hardware differences may cause issues
# Real hardware testing recommended
```

### Disassembly

```bash
make disasm
# Output: build/kernel.dis
```

## Extending

### Add USB Support
The Zero 2 W has a single micro-USB OTG port. Implementing USB requires:
- DWC2 USB controller driver
- USB stack (enumeration, descriptors)
- Device class drivers (HID, mass storage)

### Add WiFi
Requires:
- SDIO driver for CYW43439 chip
- WiFi firmware blob
- 802.11 protocol stack

### Multi-core
Currently cores 1-3 are parked. To use them:
```c
// Write entry point address to mailbox
// Each core reads its designated mailbox and jumps
```

## References

- [BCM2835 ARM Peripherals](https://www.raspberrypi.org/documentation/hardware/raspberrypi/bcm2835/BCM2835-ARM-Peripherals.pdf)
- [Raspberry Pi Firmware](https://github.com/raspberrypi/firmware)
- [ARM Cortex-A53 TRM](https://developer.arm.com/documentation/ddi0500/latest)
- [OSDev Raspberry Pi Bare Bones](https://wiki.osdev.org/Raspberry_Pi_Bare_Bones)

## License

MIT License - Do whatever you want with it.
