#!/bin/bash

echo "Testing MMRY UI Display Fix"
echo "============================="
echo

# Start the application
echo "Starting MMRY..."
./target/debug/mmry &
MMRY_PID=$!

# Wait for it to initialize
sleep 3

echo "Application started with PID: $MMRY_PID"

# Check if it's still running
if ps -p $MMRY_PID > /dev/null; then
    echo "✅ Application is running"
    echo
    echo "The window should now be visible with the clipboard interface."
    echo "You should be able to:"
    echo "  - See the clipboard items list"
    echo "  - Use j/k to navigate"
    echo "  - Press Ctrl+Alt+C to toggle visibility"
    echo "  - Press Escape to close"
    echo
    echo "Press any key to stop the test..."
    read -n 1 -s
else
    echo "❌ Application failed to start"
    exit 1
fi

# Clean up
echo
echo "Stopping application..."
kill $MMRY_PID
sleep 2

if ! ps -p $MMRY_PID > /dev/null; then
    echo "✅ Application stopped successfully"
else
    kill -9 $MMRY_PID
    echo "✅ Application force stopped"
fi

echo "Test completed!"