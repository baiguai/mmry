#!/bin/bash

# MMRY Windows Static Build Script
# Creates a statically linked executable with no DLL dependencies

set -e

echo "MMRY Windows Static Build Script"
echo
echo

# Check if MinGW-w64 is available
if ! command -v x86_64-w64-mingw32-g++ &> /dev/null; then
    echo "MinGW-w64 not found!"
    echo "Install with: sudo apt install mingw-w64"
    exit 1
fi

# Clean previous build
BUILD_DIR="build-windows-static"
if [ -d "$BUILD_DIR" ]; then
    echo "Cleaning existing build directory..."
    rm -rf "$BUILD_DIR"
fi

echo "Creating static build directory..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure for static linking
echo "Configuring for static Windows build..."
cmake .. \
    -DCMAKE_SYSTEM_NAME=Windows \
    -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
    -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
    -DCMAKE_RC_COMPILER=x86_64-w64-mingw32-windres \
    -DCMAKE_FIND_ROOT_PATH=/usr/x86_64-w64-mingw32 \
    -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER \
    -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
    -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
    -DCMAKE_CXX_FLAGS="-D_WIN32 -DWIN32 -D__WIN32__ -static-libgcc -static-libstdc++" \
    -DCMAKE_C_FLAGS="-D_WIN32 -DWIN32 -D__WIN32__ -static-libgcc -static-libstdc++" \
    -DCMAKE_EXE_LINKER_FLAGS="-static -static-libgcc -static-libstdc++"

if [ $? -ne 0 ]; then
    echo "CMake configuration failed!"
    exit 1
fi

# Build
echo "Building static Windows executable..."
make -j$(nproc 2>/dev/null || echo 4) VERBOSE=1

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

# Check output
if [ -f "bin/mmry_cpp.exe" ]; then
    echo "Static build successful!"
    echo "Static executable: $(pwd)/bin/mmry_cpp.exe"
    echo "File size: $(du -h bin/mmry_cpp.exe | cut -f1)"
    echo
    
    # Check dependencies
    echo "Checking dependencies:"
    REQUIRED_DLLS=$(x86_64-w64-mingw32-objdump -p bin/mmry_cpp.exe | grep "DLL Name" | awk '{print $3}' | sort -u)
    
    if [ -z "$REQUIRED_DLLS" ]; then
        echo "  No external DLL dependencies!"
    else
        echo "  DLL dependencies:"
        for dll in $REQUIRED_DLLS; do
            if [[ "$dll" == KERNEL32.dll ]] || [[ "$dll" == USER32.dll ]] || [[ "$dll" == GDI32.dll ]] || [[ "$dll" == msvcrt.dll ]] || [[ "$dll" == SHELL32.dll ]]; then
                echo "    $dll (Windows system DLL)"
            else
                echo "    $dll (external dependency)"
            fi
        done
    fi
    
    echo
    echo "This executable should run on any Windows machine without additional DLLs!"
    echo "Copy bin/mmry_cpp.exe to Windows and run it directly."
    
else
    echo "Build failed - executable not found!"
    exit 1
fi