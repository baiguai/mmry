#!/bin/bash

echo "=== MMRY Filter Functionality Test ==="
echo

# Start MMRY in background
echo "Starting MMRY..."
./mmry.sh &
MMRY_PID=$!
sleep 2

echo "Application started with PID: $MMRY_PID"
echo
echo "Manual test instructions:"
echo "1. Press Ctrl+Alt+C to show the window"
echo "2. Press '/' to enter filter mode"
echo "3. Type 'a' to filter for items containing 'a'"
echo "4. Verify vim keys (j/k/g/G) don't work in filter mode"
echo "5. Press Escape to exit filter mode"
echo "6. Verify vim keys work again after exiting filter mode"
echo "7. Press Q to quit the application"
echo
echo "Expected behavior:"
echo "- In filter mode: Only text input, Backspace, Enter, Escape work"
echo "- Outside filter mode: All vim navigation keys work"
echo "- Escape exits filter mode but keeps window open"
echo "- Shift+Q is disabled in filter mode (can type 'q' for filtering)"
echo "- Filtering is case-insensitive"
echo
echo "To stop test: kill $MMRY_PID"

# Wait for user interrupt
trap "echo 'Stopping MMRY...'; kill $MMRY_PID; exit" INT
wait $MMRY_PID