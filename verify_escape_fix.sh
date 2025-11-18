#!/bin/bash

echo "=== Escape Key Fix Verification ==="
echo

echo "Starting MMRY..."
./mmry.sh &
MMRY_PID=$!
sleep 2

echo "MMRY started with PID: $MMRY_PID"
echo
echo "Test the Escape key behavior:"
echo "1. Press Ctrl+Alt+C to show window"
echo "2. Press '/' to enter filter mode"
echo "3. Type something"
echo "4. Press Escape - should exit filter mode, window stays OPEN"
echo "5. Press Escape again - should hide window"
echo "6. Press Ctrl+Alt+C to show window again"
echo "7. Press Escape (not in filter mode) - should hide window"
echo
echo "If all steps work correctly, the fix is successful!"
echo
echo "Press Ctrl+C to stop test"
echo "Or run: kill $MMRY_PID"

trap "echo 'Stopping MMRY...'; kill $MMRY_PID; exit" INT
wait $MMRY_PID