#!/bin/bash

echo "=== Filter Mode Navigation Test ==="
echo

# Add test items
echo "Adding test items..."
echo "Apple banana cherry" | xclip -selection clipboard
sleep 0.5
echo "Dog elephant fish" | xclip -selection clipboard  
sleep 0.5
echo "Goat horse iguana" | xclip -selection clipboard
sleep 0.5

echo "Test items added. Starting MMRY..."
./mmry.sh &
MMRY_PID=$!
sleep 2

echo "MMRY started with PID: $MMRY_PID"
echo
echo "Test filter navigation:"
echo "1. Press Ctrl+Alt+C to show window"
echo "2. Press '/' to enter filter mode"
echo "3. Type 'a' (should show items with 'a')"
echo "4. Press Up/Down arrows to navigate filtered results"
echo "5. Press Enter on selected item (should copy to clipboard and hide window)"
echo "6. Press Escape to exit filter mode without copying"
echo
echo "Expected behavior:"
echo "- Arrow keys navigate filtered results"
echo "- Enter copies selected item and hides window"
echo "- Escape exits filter mode, keeps window open"
echo "- Vim keys (j/k) don't work in filter mode"
echo
echo "Press Ctrl+C to stop test"
echo "Or run: kill $MMRY_PID"

trap "echo 'Stopping MMRY...'; kill $MMRY_PID; exit" INT
wait $MMRY_PID