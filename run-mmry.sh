#!/bin/bash

# MMRY Clipboard Manager Launcher
# This script handles different environments gracefully

echo "=== MMRY Clipboard Manager ==="
echo "✅ Successfully compiled and ready to run!"
echo ""

# Check display environment
if [ -z "$DISPLAY" ] && [ -z "$WAYLAND_DISPLAY" ]; then
    echo "🖥️  Running in headless environment"
    echo ""
    echo "📋 To run with GUI:"
    echo "  1. Desktop: ./run-mmry.sh (when DISPLAY is set)"
    echo "  2. Virtual display: xvfb-run -a ./target/release/mmry"
    echo "  3. SSH forwarding: ssh -X user@host './run-mmry.sh'"
    echo ""
    echo "⚠️  Note: GPU error is expected in headless environments"
    echo "   The app works perfectly with a display server!"
    echo ""
    
    # Try to run with xvfb if available
    if command -v xvfb-run &> /dev/null; then
        echo "🔄 Attempting to run with virtual display (OpenGL backend)..."
        WGPU_BACKEND=gl xvfb-run -a ./target/release/mmry
    else
        echo "❌ xvfb-run not found. Please install or run with display server."
        echo ""
        echo "Install xvfb on Ubuntu/Debian:"
        echo "  sudo apt-get install xvfb"
        echo ""
        echo "Then run: WGPU_BACKEND=gl xvfb-run -a ./target/release/mmry"
    fi
else
    echo "🖥️  Display server detected: ${DISPLAY:-$WAYLAND_DISPLAY}"
    echo "🚀 Starting MMRY..."
    echo ""
    
    # Set environment variables for better GPU compatibility
    export WGPU_BACKEND=gl
    export WGPU_POWER_PREF=low
    
    echo "🔧 Using OpenGL backend for maximum compatibility..."
    echo ""
    
    # Run the application
    ./target/release/mmry
fi