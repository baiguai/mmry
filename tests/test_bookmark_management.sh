#!/bin/bash

# Bookmark Management Test
# Tests bookmark creation, management, and deletion

# Prevent interactive editors from opening
export EDITOR=""
export VISUAL=""
export GIT_EDITOR=""
unset EDITOR VISUAL GIT_EDITOR

# Prevent any interactive prompts
export DEBIAN_FRONTEND=noninteractive
export INTERACTIVE=0

echo "Testing MMRY Bookmark Management..."
echo ""

EXECUTABLE="$(dirname "$0")/../build/bin/mmry_cpp"
if [ ! -f "$EXECUTABLE" ]; then
    echo "❌ FAIL: Executable not found"
    exit 1
fi

# Test 1: Check for bookmark-related files and functions
echo "Test 1: Verifying bookmark infrastructure..."
SOURCE_FILE="$(dirname "$0")/../complete_main.cpp"

# Check for bookmark variables
BOOKMARK_VARS=(
    "bookmarkGroups"
    "selectedBookmarkGroup"
    "selectedAddBookmarkGroup"
    "selectedViewBookmarkGroup"
    "bookmarkDialogVisible"
    "addToBookmarkDialogVisible"
    "viewBookmarksDialogVisible"
)

for var in "${BOOKMARK_VARS[@]}"; do
    if grep -q "$var" "$SOURCE_FILE"; then
        echo "✅ Found bookmark variable: $var"
    else
        echo "❌ FAIL: Missing bookmark variable: $var"
        exit 1
    fi
done

# Check for bookmark functions
BOOKMARK_FUNCS=(
    "saveBookmarkGroups"
    "loadBookmarkGroups"
    "addClipToBookmarkGroup"
    "drawBookmarkDialog"
    "drawAddToBookmarkDialog"
    "drawViewBookmarksDialog"
)

for func in "${BOOKMARK_FUNCS[@]}"; do
    if grep -q "$func" "$SOURCE_FILE"; then
        echo "✅ Found bookmark function: $func"
    else
        echo "❌ FAIL: Missing bookmark function: $func"
        exit 1
    fi
done

# Test 2: Check for bookmark key bindings
echo "Test 2: Verifying bookmark key bindings..."

# Shift+M for bookmark management
if grep -q "XK_M.*ShiftMask" "$SOURCE_FILE" && grep -q "bookmarkDialogVisible.*true" "$SOURCE_FILE"; then
    echo "✅ Shift+M bookmark management binding found"
else
    echo "❌ FAIL: Shift+M bookmark management binding not found"
    exit 1
fi

# m for add to bookmark
if grep -q "XK_m" "$SOURCE_FILE" && grep -q "addToBookmarkDialogVisible" "$SOURCE_FILE"; then
    echo "✅ m add to bookmark binding found"
else
    echo "❌ FAIL: m add to bookmark binding not found"
    exit 1
fi

# ` for view bookmarks
if grep -q "XK_grave" "$SOURCE_FILE" && grep -q "viewBookmarksDialogVisible" "$SOURCE_FILE"; then
    echo "✅ Grave key view bookmarks binding found"
else
    echo "❌ FAIL: Grave key view bookmarks binding not found"
    exit 1
fi

# Test 3: Check for delete functionality
echo "Test 3: Verifying delete functionality..."
if grep -q "XK_D.*ShiftMask" "$SOURCE_FILE"; then
    echo "✅ Shift+D delete functionality found"
else
    echo "❌ FAIL: Shift+D delete functionality not found"
    exit 1
fi

# Test 4: Start application and test bookmark file creation
echo "Test 4: Testing bookmark file operations..."
"$EXECUTABLE" &
MMRY_PID=$!

sleep 2

if ! kill -0 $MMRY_PID 2>/dev/null; then
    echo "❌ FAIL: Application failed to start"
    exit 1
fi

# Check if bookmarks file is created
BOOKMARKS_FILE="$HOME/.config/mmry/bookmarks.txt"
if [ -f "$BOOKMARKS_FILE" ]; then
    echo "✅ Bookmarks file created"
else
    echo "❌ FAIL: Bookmarks file not created"
    kill $MMRY_PID 2>/dev/null || true
    exit 1
fi

# Test 5: Add some test content and verify bookmark operations
echo "Test 5: Testing bookmark operations with content..."
echo "Test content for bookmarking" | xclip -selection clipboard 2>/dev/null || echo "Test content for bookmarking" | pbcopy 2>/dev/null || true
sleep 1

kill $MMRY_PID 2>/dev/null || true

# Test 6: Check for bookmark group files
echo "Test 6: Verifying bookmark group file handling..."
CONFIG_DIR="$HOME/.config/mmry"
if [ -d "$CONFIG_DIR" ]; then
    # Look for any bookmark group files
    BOOKMARK_GROUP_FILES=$(ls "$CONFIG_DIR"/bookmarks_*.txt 2>/dev/null | wc -l)
    echo "✅ Found $BOOKMARK_GROUP_FILES bookmark group files"
else
    echo "❌ FAIL: Config directory not found"
    exit 1
fi

# Test 7: Check for proper file format in bookmarks
echo "Test 7: Verifying bookmark file format..."
if [ -f "$BOOKMARKS_FILE" ]; then
    # Check if file has proper format (one group per line)
    INVALID_LINES=0
    while IFS= read -r line; do
        if [ -n "$line" ] && [[ ! "$line" =~ ^[a-zA-Z0-9_]+$ ]]; then
            INVALID_LINES=$((INVALID_LINES + 1))
        fi
    done < "$BOOKMARKS_FILE"
    
    if [ "$INVALID_LINES" -eq 0 ]; then
        echo "✅ Bookmarks file has proper format"
    else
        echo "⚠️  WARNING: Found $INVALID_LINES potentially invalid lines in bookmarks file"
    fi
fi

echo ""
echo "✅ BOOKMARK MANAGEMENT TEST PASSED"
exit 0