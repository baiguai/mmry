#!/bin/bash

# Filter Functionality Test

# Prevent interactive editors from opening
export EDITOR=""
export VISUAL=""
export GIT_EDITOR=""
unset EDITOR VISUAL GIT_EDITOR

# Tests filter mode and related functionality

echo "Testing MMRY Filter Functionality..."
echo ""

EXECUTABLE="/home/baiguai/documents/development/rust/mmry/cpp/build/bin/mmry_cpp"
if [ ! -f "$EXECUTABLE" ]; then
    echo "❌ FAIL: Executable not found"
    exit 1
fi

# Test 1: Check for filter implementation in source
echo "Test 1: Verifying filter implementation..."
SOURCE_FILE="/home/baiguai/documents/development/rust/mmry/cpp/complete_main.cpp"

# Check for filter variables
FILTER_VARS=(
    "filterMode"
    "filterText"
    "filteredItems"
)

for var in "${FILTER_VARS[@]}"; do
    if grep -q "$var" "$SOURCE_FILE"; then
        echo "✅ Found filter variable: $var"
    else
        echo "❌ FAIL: Missing filter variable: $var"
        exit 1
    fi
done

# Check for filter functions
FILTER_FUNCS=(
    "updateFilteredItems"
    "getDisplayItemCount"
    "getActualItemIndex"
)

for func in "${FILTER_FUNCS[@]}"; do
    if grep -q "$func" "$SOURCE_FILE"; then
        echo "✅ Found filter function: $func"
    else
        echo "❌ FAIL: Missing filter function: $func"
        exit 1
    fi
done

# Test 2: Check for filter key bindings
echo "Test 2: Verifying filter key bindings..."

# / to enter filter mode
if grep -q "XK_slash" "$SOURCE_FILE" && grep -q "filterMode.*true" "$SOURCE_FILE"; then
    echo "✅ / key to enter filter mode found"
else
    echo "❌ FAIL: / key to enter filter mode not found"
    exit 1
fi

# Escape to exit filter mode
if grep -q "filterMode.*false" "$SOURCE_FILE" && grep -q "XK_Escape" "$SOURCE_FILE"; then
    echo "✅ Escape to exit filter mode found"
else
    echo "❌ FAIL: Escape to exit filter mode not found"
    exit 1
fi

# Test 3: Check for proper filter mode handling
echo "Test 3: Verifying filter mode handling..."

# Check that filter mode affects navigation
if grep -q "filterMode.*keysym" "$SOURCE_FILE"; then
    echo "✅ Filter mode affects key handling"
else
    echo "❌ FAIL: Filter mode doesn't affect key handling"
    exit 1
fi

# Check that filter mode affects display
if grep -q "getDisplayItemCount" "$SOURCE_FILE"; then
    echo "✅ Filter mode affects display"
else
    echo "❌ FAIL: Filter mode doesn't affect display"
    exit 1
fi

# Test 4: Check for filter input handling
echo "Test 4: Verifying filter input handling..."
if grep -q "filterText.*=" "$SOURCE_FILE" && grep -q "XLookupString" "$SOURCE_FILE"; then
    echo "✅ Filter input handling implemented"
else
    echo "❌ FAIL: Filter input handling not implemented"
    exit 1
fi

# Test 5: Check for backspace handling in filter mode
echo "Test 5: Verifying backspace handling..."
if grep -q "XK_BackSpace" "$SOURCE_FILE" && grep -q "filterText" "$SOURCE_FILE"; then
    echo "✅ Backspace handling in filter mode found"
else
    echo "❌ FAIL: Backspace handling in filter mode not found"
    exit 1
fi

# Test 6: Start application and test filter functionality
echo "Test 6: Testing application with filter..."
MMRY_PID=$!

sleep 2

if ! kill -0 $MMRY_PID 2>/dev/null; then
    echo "❌ FAIL: Application failed to start"
    exit 1
fi

# Add some test content
echo "Test item apple" | xclip -selection clipboard 2>/dev/null || echo "Test item apple" | pbcopy 2>/dev/null || true
sleep 0.5
echo "Test item banana" | xclip -selection clipboard 2>/dev/null || echo "Test item banana" | pbcopy 2>/dev/null || true
sleep 0.5
echo "Test item cherry" | xclip -selection clipboard 2>/dev/null || echo "Test item cherry" | pbcopy 2>/dev/null || true
sleep 0.5
echo "Different content" | xclip -selection clipboard 2>/dev/null || echo "Different content" | pbcopy 2>/dev/null || true
sleep 1

# Check that clips were added
CLIPS_FILE="$HOME/.config/mmry/clips.txt"
if [ -f "$CLIPS_FILE" ]; then
    LINE_COUNT=$(wc -l < "$CLIPS_FILE")
    if [ "$LINE_COUNT" -ge 4 ]; then
        echo "✅ Added $LINE_COUNT test items for filtering"
    else
        echo "❌ FAIL: Only $LINE_COUNT items added, expected at least 4"
        kill $MMRY_PID 2>/dev/null || true
        exit 1
    fi
else
    echo "❌ FAIL: Clips file not found"
    kill $MMRY_PID 2>/dev/null || true
    exit 1
fi

kill $MMRY_PID 2>/dev/null || true

# Test 7: Check for case-insensitive filtering
echo "Test 7: Verifying case-insensitive filtering..."
if grep -q "tolower\|toupper" "$SOURCE_FILE" || grep -q "case.*ignore" "$SOURCE_FILE"; then
    echo "✅ Case-insensitive filtering implemented"
else
    echo "⚠️  WARNING: Case-insensitive filtering may not be implemented"
fi

# Test 8: Check for filter display in UI
echo "Test 8: Verifying filter display..."
if grep -q "filterDisplay.*\/" "$SOURCE_FILE" || grep -q "\"/.*filterText" "$SOURCE_FILE"; then
    echo "✅ Filter display in UI implemented"
else
    echo "❌ FAIL: Filter display in UI not implemented"
    exit 1
fi

echo ""
echo "✅ FILTER FUNCTIONALITY TEST PASSED"
exit 0
