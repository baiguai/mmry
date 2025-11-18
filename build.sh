#!/bin/bash

# MMRY Clipboard Manager Build Script
# This script builds the C++ version of MMRY

set -e  # Exit on any error

echo "Building MMRY Clipboard Manager..."

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

# Navigate to build directory
cd build

# Configure with CMake if needed
if [ ! -f "Makefile" ]; then
    echo "Configuring with CMake..."
    cmake ..
fi

# Build the project
echo "Compiling..."
make

# Check if build was successful
if [ -f "bin/mmry_cpp" ]; then
    echo "✅ Build successful!"
    echo "Executable: $(pwd)/bin/mmry_cpp"
    echo ""
    echo "To run MMRY:"
    echo "  ./bin/mmry_cpp"
    echo ""
    echo "Or from the parent directory:"
    echo "  ./cpp/build/bin/mmry_cpp"
else
    echo "❌ Build failed!"
    exit 1
fi