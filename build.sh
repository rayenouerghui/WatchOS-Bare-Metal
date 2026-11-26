#!/bin/bash

# WatchOS Build Script
# This script compiles and runs the OS

set -e  # Exit on error

echo "================================="
echo "  WatchOS Build Script"
echo "================================="
echo ""

# Build the OS incrementally. Use `make clean` explicitly when a full rebuild is needed.
echo "[1/3] Building kernel and ISO..."
make all
echo "✓ Build complete"
echo ""

# Check if ISO was created
if [ ! -f "watch-os.iso" ]; then
    echo "✗ Error: ISO file not created!"
    exit 1
fi

echo "[2/3] ISO created successfully"
echo "✓ File: watch-os.iso"
echo ""

# Run in QEMU
echo "[3/3] Launching QEMU..."
echo "================================="
echo ""
make run
