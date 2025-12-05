#!/bin/bash

# Requirements:
# sudo apt install -y mingw-w64




# Windows build script for MMRY clipboard manager
# Compiles main.cpp and main.h into a Windows executable using MinGW-w64

set -e

echo "Building MMRY for Windows..."

# Determine build type
BUILD_TYPE_FLAGS=""
BUILD_MESSAGE="DEBUG"
if [ "$1" == "y" ]; then
    BUILD_TYPE_FLAGS="-s" # Strip all symbol tables
    BUILD_MESSAGE="RELEASE"
fi
echo "Performing $BUILD_MESSAGE build."


# Check if source files exist
if [ ! -f "src/main.cpp" ]; then
    echo "Error: src/main.cpp not found"
    exit 1
fi

if [ ! -f "src/main.h" ]; then
    echo "Error: src/main.h not found"
    exit 1
fi

# Check for MinGW-w64 compiler
if ! command -v x86_64-w64-mingw32-g++ &> /dev/null; then
    echo "Error: MinGW-w64 compiler not found"
    echo "Install with: sudo apt install mingw-w64"
    exit 1
fi

# Create build directory
mkdir -p build-windows
cd build-windows

# Compile with MinGW-w64 for Windows
echo "Compiling with MinGW-w64..."
x86_64-w64-mingw32-g++ -std=c++17 \
    -O2 \
    -Wall \
    -Wextra \
    -D_WIN32 \
    -static-libgcc \
    -static-libstdc++ \
    -static \
    $BUILD_TYPE_FLAGS \
    -o mmry.exe \
    ../src/main.cpp \
    -luser32 \
    -lgdi32 \
    -lkernel32 \
    -lwinmm

echo "Build complete: build-windows/mmry.exe"
echo "Executable size: $(du -h mmry.exe | cut -f1)"

cd ..
