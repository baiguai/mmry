#!/bin/bash

echo "=== Quick Filter Behavior Test ==="
echo

# Add test items
echo "Adding test items..."
echo "Test Item One" | xclip -selection clipboard
sleep 0.5
echo "Test Item Two" | xclip -selection clipboard  
sleep 0.5
echo "Another Item" | xclip -selection clipboard
sleep 0.5

echo "Test items added. Current clips:"
tail -3 ~/.config/mmry/clips.txt

echo
echo "Manual test checklist:"
echo "✓ Press Ctrl+Alt+C to show window"
echo "✓ Press '/' to enter filter mode"
echo "✓ Type 'test' (should show 2 items, case-insensitive)"
echo "✓ Type 'TEST' (should still show 2 items)"
echo "✓ Try vim keys j/k/g/G (should NOT work in filter mode)"
echo "✓ Try Shift+Q (should NOT quit in filter mode)"
echo "✓ Type 'q' (should work for filtering)"
echo "✓ Press Up/Down arrows to navigate filtered results"
echo "✓ Press Enter on selected item (should copy and hide window)"
echo "✓ Press Escape (should exit filter mode, window stays open)"
echo "✓ Try vim keys again (should work after exiting filter)"
echo "✓ Press Shift+Q (should quit when not in filter mode)"

echo
echo "Starting MMRY for manual testing..."
./mmry.sh &
MMRY_PID=$!

echo "MMRY started with PID: $MMRY_PID"
echo "Press Ctrl+C to stop test"
echo "Or run: kill $MMRY_PID"

trap "echo 'Stopping MMRY...'; kill $MMRY_PID; exit" INT
wait $MMRY_PID