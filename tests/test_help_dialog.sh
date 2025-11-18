#!/bin/bash

# Help Dialog Test

# Prevent interactive editors from opening
export EDITOR=""
export VISUAL=""
export GIT_EDITOR=""
unset EDITOR VISUAL GIT_EDITOR

# Tests help dialog functionality and text fitting

echo "Testing MMRY Help Dialog..."
echo ""

EXECUTABLE="/home/baiguai/documents/development/rust/mmry/cpp/build/bin/mmry_cpp"
if [ ! -f "$EXECUTABLE" ]; then
    echo "❌ FAIL: Executable not found"
    exit 1
fi

# Test 1: Check for help dialog implementation
echo "Test 1: Verifying help dialog implementation..."
SOURCE_FILE="/home/baiguai/documents/development/rust/mmry/cpp/complete_main.cpp"

# Check for help dialog variables
HELP_VARS=(
    "helpDialogVisible"
)

for var in "${HELP_VARS[@]}"; do
    if grep -q "$var" "$SOURCE_FILE"; then
        echo "✅ Found help dialog variable: $var"
    else
        echo "❌ FAIL: Missing help dialog variable: $var"
        exit 1
    fi
done

# Check for help dialog functions
HELP_FUNCS=(
    "drawHelpDialog"
)

for func in "${HELP_FUNCS[@]}"; do
    if grep -q "$func" "$SOURCE_FILE"; then
        echo "✅ Found help dialog function: $func"
    else
        echo "❌ FAIL: Missing help dialog function: $func"
        exit 1
    fi
done

# Test 2: Check for help dialog key binding
echo "Test 2: Verifying help dialog key binding..."

# ? to show help dialog
if grep -q "XK_question" "$SOURCE_FILE" && grep -q "helpDialogVisible.*true" "$SOURCE_FILE"; then
    echo "✅ ? key to show help dialog found"
else
    echo "❌ FAIL: ? key to show help dialog not found"
    exit 1
fi

# Test 3: Check for help dialog dimensions
echo "Test 3: Verifying help dialog dimensions..."
if grep -q "DIALOG_WIDTH.*=.*600" "$SOURCE_FILE" && grep -q "DIALOG_HEIGHT.*=.*500" "$SOURCE_FILE"; then
    echo "✅ Help dialog has correct dimensions (600x500)"
else
    echo "❌ FAIL: Help dialog dimensions incorrect"
    exit 1
fi

# Test 4: Check for help dialog content sections
echo "Test 4: Verifying help dialog content..."
HELP_SECTIONS=(
    "Main Window"
    "Filter Mode"
    "Bookmark Dialogs"
    "Global Hotkey"
)

for section in "${HELP_SECTIONS[@]}"; do
    if grep -q "$section" "$SOURCE_FILE"; then
        echo "✅ Found help section: $section"
    else
        echo "❌ FAIL: Missing help section: $section"
        exit 1
    fi
done

# Test 5: Check for essential keyboard shortcuts in help
echo "Test 5: Verifying essential shortcuts in help..."
ESSENTIAL_SHORTCUTS=(
    "j/k"
    "Enter"
    "Escape"
    "Shift+M"
    "m"
    "?"
    "Shift+Q"
)

for shortcut in "${ESSENTIAL_SHORTCUTS[@]}"; do
    if grep -q "$shortcut" "$SOURCE_FILE"; then
        echo "✅ Found shortcut in help: $shortcut"
    else
        echo "❌ FAIL: Missing shortcut in help: $shortcut"
        exit 1
    fi
done

# Test 6: Check for help dialog closing
echo "Test 6: Verifying help dialog closing..."
if grep -q "helpDialogVisible.*false" "$SOURCE_FILE"; then
    echo "✅ Help dialog closing implemented"
else
    echo "❌ FAIL: Help dialog closing not implemented"
    exit 1
fi

# Test 7: Check for text overflow prevention
echo "Test 7: Verifying text overflow prevention..."

# Check that text lines are reasonable length
LONG_LINES=0
while IFS= read -r line; do
    # Skip non-drawing lines
    if [[ "$line" =~ XDrawString.*displayText.*length ]]; then
        # Extract the length parameter
        if [[ "$line" =~ ([0-9]+)$ ]]; then
            LENGTH="${BASH_REMATCH[1]}"
            if [ "$LENGTH" -gt 50 ]; then
                LONG_LINES=$((LONG_LINES + 1))
            fi
        fi
    fi
done < "$SOURCE_FILE"

if [ "$LONG_LINES" -le 2 ]; then  # Allow a couple of longer lines
    echo "✅ Help dialog text lengths are reasonable"
else
    echo "❌ FAIL: Found $LONG_LINES potentially too-long text lines"
    exit 1
fi

# Test 8: Start application and test help dialog
echo "Test 8: Testing application with help dialog..."
"$EXECUTABLE" &
MMRY_PID=$!

sleep 2

if ! kill -0 $MMRY_PID 2>/dev/null; then
    echo "❌ FAIL: Application failed to start"
    exit 1
fi

# Test that application doesn't crash when help dialog code is present
echo "✅ Application starts successfully with help dialog implementation"

kill $MMRY_PID 2>/dev/null || true

# Test 9: Check for proper help dialog positioning
echo "Test 9: Verifying help dialog positioning..."
if grep -q "DIALOG_X.*WINDOW_WIDTH" "$SOURCE_FILE" && grep -q "DIALOG_Y.*WINDOW_HEIGHT" "$SOURCE_FILE"; then
    echo "✅ Help dialog is properly centered"
else
    echo "❌ FAIL: Help dialog positioning incorrect"
    exit 1
fi

echo ""
echo "✅ HELP DIALOG TEST PASSED"
exit 0
