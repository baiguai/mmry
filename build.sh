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
if [ -f "bin/mmry" ]; then
    echo "-- Build successful --"
    echo "Executable: $(pwd)/bin/mmry"
    echo ""
    echo "To run MMRY:"
    echo "  ./bin/mmry"
    echo ""
    echo "Or from the parent directory:"
    echo "  ./build/bin/mmry"
else
    echo "! failed !"
    exit 1
fi
