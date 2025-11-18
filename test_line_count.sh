#!/bin/bash

echo "=== Line Count Display Test ==="
echo

# Add test items
echo "Adding test items..."
printf "Multi-line item\nLine 2\nLine 3\nLine 4\nLine 5" | xclip -selection clipboard
sleep 0.5
echo "Single line item" | xclip -selection clipboard
sleep 0.5
printf "Another multi-line\nWith two lines" | xclip -selection clipboard

echo "Test items added. Starting MMRY..."
./mmry.sh &
MMRY_PID=$!
sleep 2

echo "MMRY started with PID: $MMRY_PID"
echo
echo "Test line counting:"
echo "1. Press Ctrl+Alt+C to show window"
echo "2. Select multi-line item and press Enter"
echo "3. Should see 'Copied X lines to clipboard'"
echo "4. Select single-line item and press Enter"
echo "5. Should see 'Copied to clipboard: content...'"
echo
echo "Expected behavior:"
echo "- Multi-line items: 'Copied X lines to clipboard'"
echo "- Single-line items: 'Copied to clipboard: content...'"
echo
echo "Press Ctrl+C to stop test"
echo "Or run: kill $MMRY_PID"

trap "echo 'Stopping MMRY...'; kill $MMRY_PID; exit" INT
wait $MMRY_PID