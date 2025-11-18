#!/bin/bash

echo "=== Testing Escape Key Fix ==="
echo

# Add test items
echo "Adding test items..."
echo "Test filter item" | xclip -selection clipboard
sleep 0.5
echo "Another test item" | xclip -selection clipboard

echo "Starting MMRY..."
./mmry.sh &
MMRY_PID=$!
sleep 2

echo "MMRY started. Test instructions:"
echo "1. Press Ctrl+Alt+C to show window"
echo "2. Press '/' to enter filter mode"
echo "3. Type 'test' (should filter items)"
echo "4. Press Escape (should exit filter mode, window stays open)"
echo "5. Press Escape again (should hide window)"
echo
echo "If step 4 exits filter mode but keeps window open, fix is working!"
echo
echo "Press Ctrl+C to stop test"
echo "Or run: kill $MMRY_PID"

trap "echo 'Stopping MMRY...'; kill $MMRY_PID; exit" INT
wait $MMRY_PID