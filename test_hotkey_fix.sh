#!/bin/bash

echo "Testing MMRY hotkey functionality..."
echo "Starting MMRY in background..."
cd /home/baiguai/documents/development/rust/mmry
./cpp/build/bin/mmry_cpp &
MMRY_PID=$!

echo "MMRY started with PID: $MMRY_PID"
echo "Waiting 2 seconds for initialization..."
sleep 2

echo "Now try pressing Ctrl+Alt+C to show the window"
echo "The window should appear with clipboard content"
echo ""
echo "Press Enter to stop MMRY and exit test..."
read

echo "Stopping MMRY..."
kill $MMRY_PID
echo "Test complete."