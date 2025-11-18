#!/bin/bash

echo "=== Final Encryption Test ==="
echo

# Kill any existing MMRY processes
pkill -f mmry_cpp 2>/dev/null || true
sleep 1

# Remove old data to start fresh
echo "Cleaning old data..."
rm -f ~/.config/mmry/clips.txt

echo "Starting MMRY with encryption..."
./mmry.sh &
MMRY_PID=$!
sleep 2

echo "Adding encrypted test content..."
echo "This should be encrypted" | xclip -selection clipboard
sleep 1

echo "Checking clips file..."
echo "Content should be encrypted (not readable plain text):"
cat ~/.config/mmry/clips.txt

echo
echo "If you see encrypted text, encryption is working!"
echo "If you see plain text, encryption failed."
echo
echo "Press Ctrl+C to stop test"
echo "Or run: kill $MMRY_PID"

trap "echo 'Stopping MMRY...'; kill $MMRY_PID; exit" INT
wait $MMRY_PID