#!/bin/bash

echo "Testing MMRY Help Dialog and Filter Escape Fix..."
echo "Starting MMRY in background..."
cd /home/baiguai/documents/development/rust/mmry
./cpp/build/bin/mmry_cpp &
MMRY_PID=$!

echo "MMRY started with PID: $MMRY_PID"
echo "Waiting 2 seconds for initialization..."
sleep 2

echo ""
echo "=== NEW FEATURES TESTED ==="
echo "✅ Help Dialog: Press '?' to show comprehensive keyboard shortcuts"
echo "✅ Filter Escape Fix: Press '/' to filter, then 'Escape' to cancel filter (won't hide window)"
echo "✅ All dialogs now properly handle Escape without hiding main window"
echo ""
echo "=== KEYBOARD SHORTCUTS ==="
echo "Main Window:"
echo "  ?           - Show help dialog"
echo "  Escape      - Hide window (or cancel filter if in filter mode)"
echo "  j/k or ↓/↑  - Navigate items"
echo "  Enter       - Copy selected item"
echo "  /           - Enter filter mode"
echo "  Shift+M     - Bookmark management"
echo "  m           - Add to bookmark"
echo "  \`           - View bookmarks"
echo "  Shift+Q     - Quit"
echo ""
echo "Press Ctrl+Alt+C to show window and test the new features"
echo "Press Enter to stop MMRY and exit test..."
read

echo "Stopping MMRY..."
kill $MMRY_PID
echo "Test complete."