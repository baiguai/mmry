#!/bin/bash

echo "=== MMRY Clipboard Manager Test ==="
echo "Testing basic functionality..."
echo

# Start the application in background
echo "1. Starting MMRY in background..."
./mmry.sh &
MMRY_PID=$!
sleep 2

echo "2. Adding test clipboard items..."
echo "Test Item A" | xclip -selection clipboard
sleep 1
echo "Test Item B" | xclip -selection clipboard  
sleep 1
echo "Test Item C" | xclip -selection clipboard
sleep 1

echo "3. Checking clips file..."
tail -3 ~/.config/mmry/clips.txt

echo
echo "4. MMRY is running with PID: $MMRY_PID"
echo "   - Press Ctrl+Alt+C to show the window"
echo "   - Use j/k to navigate, Enter to copy, Escape to hide"
echo "   - Press Q in the window to quit"
echo
echo "5. To stop test, press Ctrl+C or run: kill $MMRY_PID"

# Wait for user interrupt
trap "echo 'Stopping MMRY...'; kill $MMRY_PID; exit" INT
wait $MMRY_PID