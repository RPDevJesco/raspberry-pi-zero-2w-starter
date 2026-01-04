#!/bin/bash
#
# setup_sdcard.sh - Prepare SD card files for Pi Zero 2 W bare-metal kernel
#
# Usage: ./setup_sdcard.sh [destination_folder]
#
# This script downloads the required Raspberry Pi firmware files
# and copies everything needed to boot the custom kernel.
#

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
DEST="${1:-$SCRIPT_DIR/sdcard}"

FIRMWARE_BASE="https://github.com/raspberrypi/firmware/raw/master/boot"

echo "========================================"
echo "Pi Zero 2 W Kernel SD Card Setup"
echo "========================================"
echo ""

# Create destination folder
mkdir -p "$DEST"
echo "Destination: $DEST"
echo ""

# Download firmware files
echo "Downloading Raspberry Pi firmware..."
echo ""

for file in bootcode.bin start.elf fixup.dat; do
    if [ -f "$DEST/$file" ]; then
        echo "  [SKIP] $file (already exists)"
    else
        echo "  [GET]  $file"
        curl -sL "$FIRMWARE_BASE/$file" -o "$DEST/$file"
    fi
done

echo ""

# Copy kernel and config
echo "Copying kernel files..."
if [ -f "$SCRIPT_DIR/build/kernel8.img" ]; then
    cp "$SCRIPT_DIR/build/kernel8.img" "$DEST/"
    echo "  [COPY] kernel8.img ($(stat -c%s "$DEST/kernel8.img") bytes)"
else
    echo "  [ERROR] build/kernel8.img not found! Run 'make' first."
    exit 1
fi

if [ -f "$SCRIPT_DIR/build/config.txt" ]; then
    cp "$SCRIPT_DIR/build/config.txt" "$DEST/"
    echo "  [COPY] config.txt"
else
    cp "$SCRIPT_DIR/boot/config.txt" "$DEST/"
    echo "  [COPY] config.txt (from boot/)"
fi

echo ""
echo "========================================"
echo "SD Card files ready in: $DEST"
echo "========================================"
echo ""
ls -lh "$DEST"
echo ""
echo "Next steps:"
echo "  1. Format your SD card as FAT32"
echo "  2. Copy ALL files from $DEST to the SD card root"
echo "  3. Insert SD card into Pi Zero 2 W"
echo "  4. Connect HDMI and power on"
echo ""
