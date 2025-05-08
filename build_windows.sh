#!/bin/bash

# Check if MinGW is installed
if ! command -v x86_64-w64-mingw32-gcc &> /dev/null; then
    echo "Error: MinGW cross-compiler not found. Install it with: sudo pacman -S mingw-w64-gcc"
    exit 1
fi

# Build raylib for Windows
echo "Building raylib for Windows..."
cd lib/raylib/src
make clean
make CC=x86_64-w64-mingw32-gcc PLATFORM=PLATFORM_DESKTOP PLATFORM_OS=WINDOWS RAYLIB_LIBTYPE=STATIC

# Check if raylib was built successfully
if [ ! -f libraylib.a ]; then
    echo "Error: Failed to build raylib for Windows"
    exit 1
fi

# Go back to project root
cd ../../..

# Create build directory
mkdir -p build_windows

# Compile the project
echo "Compiling project for Windows..."
make -f Makefile.windows

# Check if compilation was successful
if [ -f build_windows/audio_visualizer.exe ]; then
    echo "Build successful! Windows executable is at: build_windows/audio_visualizer.exe"
else
    echo "Error: Failed to build Windows executable"
    exit 1
fi