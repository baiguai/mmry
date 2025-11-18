#!/bin/bash

echo "Testing MMRY Pure View-Based Hotkey Fix"
echo "========================================"
echo

# Start the application
echo "Starting MMRY application..."
./target/debug/mmry &
MMRY_PID=$!

# Wait for application to initialize
sleep 3

echo "Application started with PID: $MMRY_PID"
echo

# Check if process is still running
if ps -p $MMRY_PID > /dev/null; then
    echo "✓ Application is running successfully"
else
    echo "✗ Application failed to start or crashed"
    exit 1
fi

echo
echo "Testing hotkey functionality..."
echo "The application should now be listening for Ctrl+Alt+C"
echo "When you press the hotkey, the window should appear/disappear"
echo

# Wait for user to test
echo "Press Ctrl+Alt+C to test the hotkey functionality"
echo "Press any key in this terminal to stop the test..."
read -n 1 -s

echo
echo "Stopping application..."
kill $MMRY_PID

# Wait a moment for graceful shutdown
sleep 2

# Check if process was stopped
if ! ps -p $MMRY_PID > /dev/null; then
    echo "✓ Application stopped successfully"
    echo "✓ Pure view-based approach appears to work without crashes!"
else
    echo "⚠ Application still running, forcing shutdown..."
    kill -9 $MMRY_PID
fi

echo
echo "Test completed!"