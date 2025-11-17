#!/bin/bash

echo "Testing MMRY hotkey functionality..."

# Start MMRY in background
echo "Starting MMRY..."
cargo run &
MMRY_PID=$!

# Wait for MMRY to initialize
sleep 3

echo "MMRY started with PID: $MMRY_PID"
echo "You can now test the hotkey by pressing Ctrl+Alt+C"
echo "The application should hide/show when you press the hotkey"
echo ""
echo "Press Ctrl+C to stop this test script"

# Wait for user to stop the script
wait $MMRY_PID