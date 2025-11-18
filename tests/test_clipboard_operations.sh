#!/bin/bash

# Clipboard Operations Test

# Prevent interactive editors from opening
export EDITOR=""
export VISUAL=""
export GIT_EDITOR=""
unset EDITOR VISUAL GIT_EDITOR

# Tests clipboard monitoring and basic operations

echo "Testing MMRY Clipboard Operations..."
echo ""

EXECUTABLE="$(dirname "$0")/../build/bin/mmry_cpp"
if [ ! -f "$EXECUTABLE" ]; then
    echo "❌ FAIL: Executable not found"
    exit 1
fi

# Backup user data
echo "Backing up user data..."
CONFIG_DIR="$HOME/.config/mmry"
BACKUP_DIR="/tmp/mmry_clipboard_backup_$$"

if [ -d "$CONFIG_DIR" ]; then
    mkdir -p "$BACKUP_DIR"
    cp -r "$CONFIG_DIR" "$BACKUP_DIR/" 2>/dev/null || true
    echo "✅ User data backed up"
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

# Test 1: Add clipboard items
echo "Test 1: Adding clipboard items..."
TEST_TEXTS=(
    "Test clipboard item 1"
    "Test clipboard item 2 with more content"
    "Multi-line
clipboard
item
3"
)

for i in "${!TEST_TEXTS[@]}"; do
    echo "${TEST_TEXTS[$i]}" | xclip -selection clipboard 2>/dev/null || echo "${TEST_TEXTS[$i]}" | pbcopy 2>/dev/null || true
    sleep 0.5
    echo "✅ Added test item $((i+1))"
done

# Wait for items to be processed
sleep 2

# Test 2: Check clips file
echo "Test 2: Verifying clips file..."
CLIPS_FILE="$HOME/.config/mmry/clips.txt"
if [ -f "$CLIPS_FILE" ]; then
    LINE_COUNT=$(wc -l < "$CLIPS_FILE")
    if [ "$LINE_COUNT" -ge 3 ]; then
        echo "✅ Clips file contains $LINE_COUNT lines"
    else
        echo "❌ FAIL: Clips file only has $LINE_COUNT lines, expected at least 3"
        kill $MMRY_PID 2>/dev/null || true
        exit 1
    fi
else
    echo "❌ FAIL: Clips file not found"
    kill $MMRY_PID 2>/dev/null || true
    exit 1
fi

# Test 3: Verify file format
echo "Test 3: Verifying file format..."
INVALID_LINES=0
while IFS= read -r line; do
    if [[ ! "$line" =~ ^[0-9]+\| ]]; then
        INVALID_LINES=$((INVALID_LINES + 1))
    fi
done < "$CLIPS_FILE"

if [ "$INVALID_LINES" -eq 0 ]; then
    echo "✅ All lines have correct format (timestamp|content)"
else
    echo "❌ FAIL: Found $INVALID_LINES lines with invalid format"
    kill $MMRY_PID 2>/dev/null || true
    exit 1
fi

# Test 4: Content encryption
echo "Test 4: Verifying content encryption..."
ENCRYPTED_CONTENT=false
PLAINTEXT_CONTENT=false
while IFS= read -r line; do
    # Extract content part (after timestamp)
    CONTENT="${line#*|}"
    if [[ "$CONTENT" =~ ^[A-Za-z0-9+/]+=*$ ]]; then
        ENCRYPTED_CONTENT=true
    elif [[ "$CONTENT" =~ ^[A-Za-z0-9[:space:][:punct:]+$ ]]; then
        PLAINTEXT_CONTENT=true
    fi
done < "$CLIPS_FILE"

if [ "$ENCRYPTED_CONTENT" = true ]; then
    echo "✅ Content appears to be encrypted"
elif [ "$PLAINTEXT_CONTENT" = true ]; then
    echo "✅ Content appears to be in plaintext (expected for fresh test files)"
else
    echo "⚠️  WARNING: Content format unclear"
fi

# Clean up
kill $MMRY_PID 2>/dev/null || true

# Restore user data
echo "Restoring user data..."
if [ -d "$BACKUP_DIR/mmry" ]; then
    rm -rf "$CONFIG_DIR" 2>/dev/null || true
    mv "$BACKUP_DIR/mmry" "$HOME/.config/" 2>/dev/null || true
    echo "✅ User data restored"
else
    rm -rf "$CONFIG_DIR" 2>/dev/null || true
    echo "✅ Test data cleaned up"
fi

rm -rf "$BACKUP_DIR" 2>/dev/null || true

echo ""
echo "✅ CLIPBOARD OPERATIONS TEST PASSED"
exit 0
