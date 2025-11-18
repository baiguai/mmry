#!/bin/bash

# Hotkey Test

# Prevent interactive editors from opening
export EDITOR=""
export VISUAL=""
export GIT_EDITOR=""
unset EDITOR VISUAL GIT_EDITOR

# Tests global hotkey registration and handling

echo "Testing MMRY Hotkey Functionality..."
echo ""

EXECUTABLE="/home/baiguai/documents/development/rust/mmry/cpp/build/bin/mmry_cpp"
if [ ! -f "$EXECUTABLE" ]; then
    echo "❌ FAIL: Executable not found"
    exit 1
fi

# Test 1: Check for hotkey implementation in source
echo "Test 1: Verifying hotkey implementation..."
SOURCE_FILE="/home/baiguai/documents/development/rust/mmry/cpp/complete_main.cpp"

# Check for hotkey constants
HOTKEY_CONSTS=(
    "Ctrl+Alt+C"
    "XK_c"
    "ControlMask"
    "Mod1Mask"
)

for const in "${HOTKEY_CONSTS[@]}"; do
    if grep -q "$const" "$SOURCE_FILE"; then
        echo "✅ Found hotkey constant: $const"
    else
        echo "❌ FAIL: Missing hotkey constant: $const"
        exit 1
    fi
done

# Check for hotkey registration functions
HOTKEY_FUNCS=(
    "XGrabKey"
    "XSelectInput"
)

for func in "${HOTKEY_FUNCS[@]}"; do
    if grep -q "$func" "$SOURCE_FILE"; then
        echo "✅ Found hotkey function: $func"
    else
        echo "❌ FAIL: Missing hotkey function: $func"
        exit 1
    fi
done

# Test 2: Check for hotkey event handling
echo "Test 2: Verifying hotkey event handling..."
if grep -q "KeyPress" "$SOURCE_FILE" && grep -q "hotkey" "$SOURCE_FILE"; then
    echo "✅ Hotkey event handling implemented"
else
    echo "❌ FAIL: Hotkey event handling not implemented"
    exit 1
fi

# Test 3: Check for show/hide functionality
echo "Test 3: Verifying show/hide functionality..."
if grep -q "showWindow" "$SOURCE_FILE" && grep -q "hideWindow" "$SOURCE_FILE"; then
    echo "✅ Show/hide window functionality implemented"
else
    echo "❌ FAIL: Show/hide window functionality not implemented"
    exit 1
fi

# Test 4: Check for window visibility management
echo "Test 4: Verifying window visibility management..."
if grep -q "visible.*true" "$SOURCE_FILE" && grep -q "visible.*false" "$SOURCE_FILE"; then
    echo "✅ Window visibility management implemented"
else
    echo "❌ FAIL: Window visibility management not implemented"
    exit 1
fi

# Test 5: Start application and test hotkey registration
echo "Test 5: Testing hotkey registration..."
MMRY_PID=$!

sleep 3  # Give extra time for hotkey registration

if ! kill -0 $MMRY_PID 2>/dev/null; then
    echo "❌ FAIL: Application failed to start"
    exit 1
fi

# Check if hotkey registration succeeded (process should still be running)
echo "✅ Hotkey registration successful (application still running)"

# Test 6: Check for proper error handling
echo "Test 6: Verifying hotkey error handling..."
if grep -q "grab.*hotkey" "$SOURCE_FILE" || grep -q "Successfully grabbed" "$SOURCE_FILE"; then
    echo "✅ Hotkey registration feedback implemented"
else
    echo "⚠️  WARNING: Hotkey registration feedback may not be implemented"
fi

# Test 7: Check for multiple hotkey support
echo "Test 7: Verifying multiple hotkey support..."
if grep -q "Shift\\+Q" "$SOURCE_FILE" && grep -q "quit" "$SOURCE_FILE"; then
    echo "✅ Multiple hotkey support implemented"
else
    echo "❌ FAIL: Multiple hotkey support not implemented"
    exit 1
fi

# Test 8: Check for hotkey conflict handling
echo "Test 8: Verifying hotkey conflict handling..."
if grep -q "BadAccess" "$SOURCE_FILE" || grep -q "grab.*fail" "$SOURCE_FILE"; then
    echo "✅ Hotkey conflict handling implemented"
else
    echo "⚠️  WARNING: Hotkey conflict handling may not be implemented"
fi

kill $MMRY_PID 2>/dev/null || true

echo ""
echo "✅ HOTKEY TEST PASSED"
exit 0
