#!/bin/bash

# WatchOS Build Script
# This script compiles and runs the OS

set -e  # Exit on error

echo "================================="
echo "  WatchOS Build Script"
echo "================================="
echo ""

# Clean previous build
echo "[1/4] Cleaning previous build..."
make clean
echo "✓ Clean complete"
echo ""

# Build the OS
echo "[2/4] Building kernel..."
make all
echo "✓ Build complete"
echo ""

# Check if ISO was created
if [ ! -f "watch-os.iso" ]; then
    echo "✗ Error: ISO file not created!"
    exit 1
fi

echo "[3/4] ISO created successfully"
echo "✓ File: watch-os.iso"
echo ""

# Run in QEMU
echo "[4/4] Launching QEMU..."
echo "================================="
echo ""
make run
