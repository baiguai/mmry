#!/bin/bash

echo "=== Line Number Display Test ==="
echo

# Start MMRY first
echo "Starting MMRY..."
./mmry.sh &
MMRY_PID=$!
sleep 2

echo "MMRY started. Adding multi-line content..."
printf "Test multi-line\nSecond line\nThird line" | xclip -selection clipboard
sleep 1

echo "Adding single-line content..."
echo "Single line test" | xclip -selection clipboard
sleep 1

echo "Content added. Current clips:"
tail -3 ~/.config/mmry/clips.txt

echo
echo "Test instructions:"
echo "1. Press Ctrl+Alt+C to show window"
echo "2. Look for multi-line items - should show '(X lines)'"
echo "3. Single-line items should show without line count"
echo
echo "If line numbers are visible, feature is working!"
echo
echo "Press Ctrl+C to stop test"
echo "Or run: kill $MMRY_PID"

trap "echo 'Stopping MMRY...'; kill $MMRY_PID; exit" INT
wait $MMRY_PID