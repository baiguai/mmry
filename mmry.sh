#!/bin/bash

# MMRY Clipboard Manager Launcher
# Run from git root folder

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CPP_DIR="$SCRIPT_DIR/cpp"
BUILD_DIR="$CPP_DIR/build"
EXECUTABLE="$BUILD_DIR/bin/mmry_cpp"

# Check if the executable exists
if [ ! -f "$EXECUTABLE" ]; then
    echo "MMRY executable not found. Building..."
    cd "$CPP_DIR"
    
    # Create build directory if it doesn't exist
    mkdir -p build
    cd build
    
    # Build the application
    cmake .. && make
    
    if [ $? -ne 0 ]; then
        echo "Build failed!"
        exit 1
    fi
    
    echo "Build successful!"
fi

# Run the application
echo "Starting MMRY Clipboard Manager..."
echo "Press Ctrl+Alt+C to show/hide window"
echo "Press Q in the window to quit"
echo ""

cd "$SCRIPT_DIR"
exec "$EXECUTABLE"