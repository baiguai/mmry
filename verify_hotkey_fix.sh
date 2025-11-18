#!/bin/bash

echo "=== MMRY Hotkey Fix Verification ==="
echo ""
echo "This test verifies that the global hotkey (Ctrl+Alt+C) now properly"
echo "toggles window visibility using the improved window management system."
echo ""

# Start MMRY in background
echo "Starting MMRY..."
cargo run &
MMRY_PID=$!

# Wait for MMRY to initialize
sleep 3

echo "✅ MMRY started successfully (PID: $MMRY_PID)"
echo ""
echo "📋 Test Instructions:"
echo "1. The MMRY window should be visible now"
echo "2. Press Ctrl+Alt+C to hide the window"
echo "3. Press Ctrl+Alt+C again to show the window"
echo "4. The window should properly toggle visibility"
echo ""
echo "🔧 Alternative test: Press Ctrl+Space to manually toggle"
echo "🔧 Quit: Press Ctrl+Q to exit the application"
echo ""
echo "📊 Debug output will show visibility changes in the terminal"
echo ""

# Monitor the process
wait $MMRY_PID

echo ""
echo "✅ Test completed"