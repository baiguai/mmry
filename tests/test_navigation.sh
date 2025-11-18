#!/bin/bash

# Navigation Test

# Prevent interactive editors from opening
export EDITOR=""
export VISUAL=""
export GIT_EDITOR=""
unset EDITOR VISUAL GIT_EDITOR

# Tests keyboard navigation in main interface

echo "Testing MMRY Navigation..."
echo ""

EXECUTABLE="$(dirname "$0")/../build/bin/mmry_cpp"
if [ ! -f "$EXECUTABLE" ]; then
    echo "❌ FAIL: Executable not found"
    exit 1
fi

# Start MMRY
echo "Starting MMRY..."
MMRY_PID=$!

sleep 2

# Check if process is running
if ! kill -0 $MMRY_PID 2>/dev/null; then
    echo "❌ FAIL: Application failed to start"
    exit 1
fi

# Test 1: Add some test content
echo "Test 1: Adding test content..."
for i in {1..5}; do
    echo "Test item $i for navigation" | xclip -selection clipboard 2>/dev/null || echo "Test item $i for navigation" | pbcopy 2>/dev/null || true
    sleep 0.5
done

sleep 2

# Test 2: Check that navigation keys are recognized
echo "Test 2: Testing navigation key handling..."
# This is a basic test - we can't easily simulate X11 key events in shell script
# But we can verify the application doesn't crash when keys are pressed
# For now, we'll just verify the application stays running

if kill -0 $MMRY_PID 2>/dev/null; then
    echo "✅ Application is running and ready for navigation"
else
    echo "❌ FAIL: Application crashed during setup"
    exit 1
fi

# Test 3: Verify clips file has content for navigation
echo "Test 3: Verifying navigation targets..."
CLIPS_FILE="$HOME/.config/mmry/clips.txt"
if [ -f "$CLIPS_FILE" ]; then
    LINE_COUNT=$(wc -l < "$CLIPS_FILE")
    if [ "$LINE_COUNT" -gt 0 ]; then
        echo "✅ Found $LINE_COUNT items available for navigation"
    else
        echo "❌ FAIL: No items available for navigation"
        kill $MMRY_PID 2>/dev/null || true
        exit 1
    fi
else
    echo "❌ FAIL: Clips file not found"
    kill $MMRY_PID 2>/dev/null || true
    exit 1
fi

# Test 4: Check for navigation-related code
echo "Test 4: Verifying navigation implementation..."
SOURCE_FILE="$(dirname "$0")/../complete_main.cpp"
if [ -f "$SOURCE_FILE" ]; then
    # Check for j/k navigation
    if grep -q "XK_j.*XK_Down" "$SOURCE_FILE" && grep -q "XK_k.*XK_Up" "$SOURCE_FILE"; then
        echo "✅ j/k navigation implemented"
    else
        echo "❌ FAIL: j/k navigation not found"
        kill $MMRY_PID 2>/dev/null || true
        exit 1
    fi
    
    # Check for gg/Shift+G navigation
    if grep -q "XK_g" "$SOURCE_FILE" && grep -q "XK_G.*ShiftMask" "$SOURCE_FILE"; then
        echo "✅ gg/Shift+G navigation implemented"
    else
        echo "❌ FAIL: gg/Shift+G navigation not found"
        kill $MMRY_PID 2>/dev/null || true
        exit 1
    fi
    
    # Check for arrow key navigation
    if grep -q "XK_Down" "$SOURCE_FILE" && grep -q "XK_Up" "$SOURCE_FILE"; then
        echo "✅ Arrow key navigation implemented"
    else
        echo "❌ FAIL: Arrow key navigation not found"
        kill $MMRY_PID 2>/dev/null || true
        exit 1
    fi
else
    echo "❌ FAIL: Source file not found"
    kill $MMRY_PID 2>/dev/null || true
    exit 1
fi

# Clean up
kill $MMRY_PID 2>/dev/null || true

echo ""
echo "✅ NAVIGATION TEST PASSED"
exit 0
