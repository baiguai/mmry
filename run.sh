#!/bin/bash

# MMRY Clipboard Manager Run Script
# This script builds and runs the C++ version of MMRY

set -e  # Exit on any error

echo "🚀 Starting MMRY Clipboard Manager..."

# First build the application
echo "Building MMRY..."
./build.sh

# Check if build was successful
if [ ! -f "build/bin/mmry" ]; then
    echo "❌ Build failed! Cannot start MMRY."
    exit 1
fi

echo "✅ Build successful!"
echo ""
echo "Starting MMRY..."
echo "📋 Clipboard Manager Features:"
echo "  • Press Ctrl+Alt+C to show window"
echo "  • Press ? for help dialog"
echo "  • Press Shift+M for bookmark management"
echo "  • Press / to filter clipboard items"
echo "  • Press Escape to hide window"
echo "  • Press Shift+Q to quit"
echo ""
echo "🔧 Hotkey: Ctrl+Alt+C (fallback: Ctrl+Alt+V)"
echo "📁 Config: ~/.config/mmry"
echo ""
echo "Starting application..."
echo "----------------------------------------"

# Run the application
./build/bin/mmry
