# Pi Zero 2 W Bare-Metal Kernel

A minimal bare-metal kernel for the Raspberry Pi Zero 2 W that boots without any operating system and displays hardware specifications on HDMI output.

## What It Does

This kernel boots directly on the ARM Cortex-A53 processor, bypassing Linux entirely, and:

- **Initializes the HDMI framebuffer** via the VideoCore mailbox interface
- **Queries hardware information** including board revision, serial number, clock speeds, and memory configuration
- **Renders text** using an 8x8 bitmap font in classic green-on-black terminal style
- **Provides LED feedback** via the onboard ACT LED for debugging

### Displayed Information

| Section | Data |
|---------|------|
| Board | Model name, revision code, serial number, firmware version |
| Processor | SoC model, CPU cores, architecture, ARM/core clock speeds |
| Memory | ARM memory size/base, GPU memory size/base, SDRAM clock |
| Network | WiFi/Bluetooth specs, MAC address |
| Display | Resolution, color depth, pitch, framebuffer address |

## Hardware Requirements

- **Raspberry Pi Zero 2 W** (BCM2710A1 / Cortex-A53)
- **MicroSD card** (any size, formatted FAT32)
- **Mini HDMI cable** and display
- **5V micro-USB power supply**

This kernel is specifically written for the Pi Zero 2 W. It will not work on Pi 4/5 (different peripheral base address) or Pi Zero/Zero W (32-bit ARM11).

## Building

### Prerequisites

Install the AArch64 cross-compiler:

```bash
# Ubuntu/Debian
sudo apt install gcc-aarch64-linux-gnu

# Or download ARM GNU Toolchain from:
# https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads
```

### Compile

```bash
make
```

Output:
```
build/kernel8.img   - Kernel binary (≈10KB)
build/config.txt    - Boot configuration
```

### Other Make Targets

```bash
make clean      # Remove build artifacts
make size       # Show section sizes
make disasm     # Generate disassembly
```

## Deployment

### Quick Setup

Run the included script to download firmware and prepare all files:

```bash
./setup_sdcard.sh
```

This creates an `sdcard/` folder with everything needed.

### Manual Setup

1. **Format SD card as FAT32**

2. **Download Raspberry Pi firmware** from [raspberrypi/firmware](https://github.com/raspberrypi/firmware/tree/master/boot):
   - `bootcode.bin`
   - `start.elf`
   - `fixup.dat`

3. **Copy files to SD card root:**
   ```
   SD Card (FAT32)/
   ├── bootcode.bin    (from Pi firmware)
   ├── start.elf       (from Pi firmware)
   ├── fixup.dat       (from Pi firmware)
   ├── config.txt      (from build/)
   └── kernel8.img     (from build/)
   ```

4. **Insert SD card**, connect HDMI, then power on

### LED Indicators

| Pattern | Meaning |
|---------|---------|
| 1 blink | Kernel started |
| 2 blinks | Framebuffer initialized |
| 3 blinks | Drawing to screen |
| Slow heartbeat | Success - display should be active |
| Rapid 5-blink bursts | Framebuffer initialization failed |

## Project Structure

```
pi-kernel/
├── Makefile                 # Build configuration
├── linker.ld                # Memory layout (kernel @ 0x80000)
├── setup_sdcard.sh          # Firmware download helper
│
├── boot/
│   └── config.txt           # GPU bootloader configuration
│
├── include/
│   ├── types.h              # uint32_t, bool, etc.
│   ├── gpio.h               # BCM2710 peripheral addresses
│   ├── mailbox.h            # VideoCore mailbox protocol
│   ├── framebuffer.h        # HDMI framebuffer interface
│   ├── font8x8.h            # Bitmap font data
│   ├── sysinfo.h            # Hardware query interface
│   ├── string.h             # String utilities
│   └── led.h                # ACT LED control
│
├── src/
│   ├── boot.S               # AArch64 entry point
│   ├── drivers/
│   │   ├── mailbox.c        # Mailbox read/write/call
│   │   └── framebuffer.c    # FB init, pixel/text drawing
│   ├── kernel/
│   │   ├── kernel.c         # Main entry, display rendering
│   │   └── sysinfo.c        # Hardware info queries
│   └── lib/
│       └── string.c         # memset, strcpy, itoa, etc.
│
└── build/                   # Compiled output
```

## Technical Details

### Memory Map (Pi Zero 2 W)

| Address | Region |
|---------|--------|
| `0x00000000` | ARM memory base |
| `0x00080000` | Kernel load address |
| `0x1C000000` | VideoCore GPU memory (with 128MB split) |
| `0x3F000000` | Peripheral registers |
| `0x3F00B880` | Mailbox interface |
| `0x3F200000` | GPIO registers |

### Boot Sequence

```
GPU Power On
    │
    ▼
bootcode.bin (GPU first stage)
    │
    ▼
start.elf (GPU firmware, reads config.txt)
    │
    ▼
kernel8.img loaded to 0x80000
    │
    ▼
ARM cores released from reset
    │
    ▼
_start (boot.S)
    ├── Core 0: Clear BSS → kernel_main()
    └── Cores 1-3: WFE loop (parked)
```

### Mailbox Protocol

The ARM communicates with the VideoCore GPU through a mailbox interface. Key points:

- **Address**: `0x3F00B880`
- **Channel 8**: Property tags (ARM → VC)
- **Buffer alignment**: 16 bytes
- **Bus address translation**: Clear bit 30 (`& 0x3FFFFFFF`) to convert GPU address to ARM address

#### Framebuffer Mailbox Buffer Layout

```c
[0]  = total size in bytes
[1]  = request code (0x00000000)
[2]  = 0x00048003  // TAG: Set physical W/H
[3]  = 8           // Value buffer size
[4]  = 0           // Request/response code
[5]  = width
[6]  = height
[7]  = 0x00048004  // TAG: Set virtual W/H
...
[21] = 0x00040001  // TAG: Allocate framebuffer
[22] = 8
[23] = 0
[24] = → framebuffer address (response)
[25] = → framebuffer size (response)
[26] = 0x00040008  // TAG: Get pitch
[27] = 4
[28] = 0
[29] = → pitch in bytes (response)
[30] = 0x00000000  // End tag
```

### Key Constants

```c
#define PERIPHERAL_BASE     0x3F000000
#define MAILBOX_BASE        (PERIPHERAL_BASE + 0xB880)
#define GPIO_BASE           (PERIPHERAL_BASE + 0x200000)

#define ACT_LED_PIN         29      // Active low

#define TAG_FB_ALLOC        0x00040001
#define TAG_FB_SET_PHYS_WH  0x00048003
#define TAG_FB_SET_DEPTH    0x00048005
#define TAG_FB_GET_PITCH    0x00040008
```

## Language & Standards

- **C99** with GNU extensions
- `__attribute__((aligned(16)))` for DMA buffer alignment
- `asm volatile()` for ARM intrinsics (`wfe`, `nop`)
- No standard library — all functions implemented from scratch

## Common Issues

**Black screen, solid green LED:**
- Framebuffer address read from wrong mailbox buffer index
- Check `mailbox_buffer[24]` for FB address, `[29]` for pitch

**Black screen, no LED activity:**
- SD card not FAT32 or files not at root level
- Missing firmware files (`bootcode.bin`, `start.elf`)

**Black screen, LED blinking rapidly:**
- Mailbox call failing — check `gpu_mem` in config.txt (needs ≥64)

**Kernel not loading:**
- Verify `kernel=kernel8.img` and `arm_64bit=1` in config.txt

## Extending

Ideas for further development:

- **UART console** — Add serial output for debugging (`0x3F201000`)
- **USB input** — Implement DWC2 USB controller driver
- **Multi-core** — Wake cores 1-3 via mailbox spin table
- **PWM audio** — Generate tones through headphone jack
- **GPIO control** — Blink external LEDs, read buttons

## License

MIT — Do whatever you want with it.

## Acknowledgments

- [Raspberry Pi firmware](https://github.com/raspberrypi/firmware)
- [BCM2835 ARM Peripherals](https://www.raspberrypi.org/documentation/hardware/raspberrypi/bcm2835/)
- [OSDev Wiki](https://wiki.osdev.org/Raspberry_Pi_Bare_Bones)