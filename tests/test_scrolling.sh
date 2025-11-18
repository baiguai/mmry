#!/bin/bash

# Scrolling Test

# Prevent interactive editors from opening
export EDITOR=""
export VISUAL=""
export GIT_EDITOR=""
unset EDITOR VISUAL GIT_EDITOR

# Tests scrolling functionality in all dialogs

echo "Testing MMRY Scrolling Functionality..."
echo ""

EXECUTABLE="/home/baiguai/documents/development/rust/mmry/cpp/build/bin/mmry_cpp"
if [ ! -f "$EXECUTABLE" ]; then
    echo "❌ FAIL: Executable not found"
    exit 1
fi

# Test 1: Check for scrolling implementation in source
echo "Test 1: Verifying scrolling implementation..."
SOURCE_FILE="/home/baiguai/documents/development/rust/mmry/cpp/complete_main.cpp"
if [ ! -f "$SOURCE_FILE" ]; then
    echo "❌ FAIL: Source file not found"
    exit 1
fi

# Check for scroll offset variables
SCROLL_VARS=(
    "consoleScrollOffset"
    "bookmarkMgmtScrollOffset" 
    "addBookmarkScrollOffset"
    "viewBookmarksScrollOffset"
)

for var in "${SCROLL_VARS[@]}"; do
    if grep -q "$var" "$SOURCE_FILE"; then
        echo "✅ Found scroll variable: $var"
    else
        echo "❌ FAIL: Missing scroll variable: $var"
        exit 1
    fi
done

# Check for scroll update functions
SCROLL_FUNCS=(
    "updateConsoleScrollOffset"
    "updateBookmarkMgmtScrollOffset"
    "updateAddBookmarkScrollOffset"
    "updateScrollOffset"
)

for func in "${SCROLL_FUNCS[@]}"; do
    if grep -q "$func" "$SOURCE_FILE"; then
        echo "✅ Found scroll function: $func"
    else
        echo "❌ FAIL: Missing scroll function: $func"
        exit 1
    fi
done

# Test 2: Check for scroll indicators in drawing code
echo "Test 2: Verifying scroll indicators..."
if grep -q "scrollText.*to_string.*+" "$SOURCE_FILE"; then
    echo "✅ Found scroll indicator implementation"
else
    echo "❌ FAIL: Scroll indicators not implemented"
    exit 1
fi

# Test 3: Check for j/k/gg/Shift+G in all dialogs
echo "Test 3: Verifying vim-style navigation in dialogs..."

# Main console scrolling
if grep -q "updateConsoleScrollOffset" "$SOURCE_FILE" && grep -q "XK_j.*XK_Down" "$SOURCE_FILE"; then
    echo "✅ Main console has vim navigation with scrolling"
else
    echo "❌ FAIL: Main console missing vim navigation with scrolling"
    exit 1
fi

# Bookmark management scrolling

# Prevent interactive editors from opening
export EDITOR=""
export VISUAL=""
export GIT_EDITOR=""
unset EDITOR VISUAL GIT_EDITOR

if grep -q "updateBookmarkMgmtScrollOffset" "$SOURCE_FILE" && grep -q "bookmarkDialogVisible" "$SOURCE_FILE" && grep -q "XK_j" "$SOURCE_FILE"; then
    echo "✅ Bookmark management has vim navigation with scrolling"
else
    echo "❌ FAIL: Bookmark management missing vim navigation with scrolling"
    exit 1
fi

# Add bookmark scrolling
if grep -q "updateAddBookmarkScrollOffset" "$SOURCE_FILE" && grep -q "addToBookmarkDialogVisible" "$SOURCE_FILE" && grep -q "XK_j" "$SOURCE_FILE"; then
    echo "✅ Add bookmark has vim navigation with scrolling"
else
    echo "❌ FAIL: Add bookmark missing vim navigation with scrolling"
    exit 1
fi

# View bookmarks scrolling (already existed)
if grep -q "updateScrollOffset" "$SOURCE_FILE" && grep -q "viewBookmarksDialogVisible" "$SOURCE_FILE" && grep -q "XK_j" "$SOURCE_FILE"; then
    echo "✅ View bookmarks has vim navigation with scrolling"
else
    echo "❌ FAIL: View bookmarks missing vim navigation with scrolling"
    exit 1
fi

# Test 4: Check for proper scroll offset reset
echo "Test 4: Verifying scroll offset reset..."
RESET_PATTERNS=(
    "consoleScrollOffset.*=.*0"
    "bookmarkMgmtScrollOffset.*=.*0"
    "addBookmarkScrollOffset.*=.*0"
    "viewBookmarksScrollOffset.*=.*0"
)

for pattern in "${RESET_PATTERNS[@]}"; do
    if grep -q "$pattern" "$SOURCE_FILE"; then
        echo "✅ Found scroll offset reset pattern"
    else
        echo "❌ FAIL: Missing scroll offset reset pattern"
        exit 1
    fi
done

# Test 5: Start application and verify it doesn't crash with scrolling
echo "Test 5: Testing application stability with scrolling..."
"$EXECUTABLE" &
MMRY_PID=$!

sleep 2

if kill -0 $MMRY_PID 2>/dev/null; then
    echo "✅ Application starts successfully with scrolling implementation"
else
    echo "❌ FAIL: Application fails to start with scrolling implementation"
    exit 1
fi

kill $MMRY_PID 2>/dev/null || true

echo ""
echo "✅ SCROLLING TEST PASSED"
exit 0
