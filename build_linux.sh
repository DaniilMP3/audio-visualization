#!/bin/bash

echo "Building raylib for Linux..."
cd lib/raylib/src
make clean
make PLATFORM=PLATFORM_DESKTOP

# Check if raylib was built successfully
if [ ! -f libraylib.a ]; then
    echo "Error: Failed to build raylib for Linux"
    exit 1
fi

# Go back to project root
cd ../../..

# Create build directory
mkdir -p build

# Compile the project
echo "Compiling project for Linux..."
make

# Check if compilation was successful
if [ -f build/audio_visualizer ]; then
    echo "Build successful! Linux executable is at: build/audio_visualizer"
else
    echo "Error: Failed to build Linux executable"
    exit 1
fi