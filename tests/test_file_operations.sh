#!/bin/bash

# File Operations Test

# Prevent interactive editors from opening
export EDITOR=""
export VISUAL=""
export GIT_EDITOR=""
unset EDITOR VISUAL GIT_EDITOR

# Tests file I/O operations and data persistence

echo "Testing MMRY File Operations..."
echo ""

EXECUTABLE="/home/baiguai/documents/development/rust/mmry/cpp/build/bin/mmry_cpp"
if [ ! -f "$EXECUTABLE" ]; then
    echo "❌ FAIL: Executable not found"
    exit 1
fi

# Test 1: Check for file operation functions in source
echo "Test 1: Verifying file operation implementation..."
SOURCE_FILE="/home/baiguai/documents/development/rust/mmry/cpp/complete_main.cpp"

# Check for file I/O functions
FILE_FUNCS=(
    "saveToFile"
    "loadFromFile"
    "saveBookmarkGroups"
    "loadBookmarkGroups"
)

for func in "${FILE_FUNCS[@]}"; do
    if grep -q "$func" "$SOURCE_FILE"; then
        echo "✅ Found file function: $func"
    else
        echo "❌ FAIL: Missing file function: $func"
        exit 1
    fi
done

# Test 2: Check for proper file handling
echo "Test 2: Verifying proper file handling..."
FILE_OPERATIONS=(
    "ofstream"
    "ifstream"
    "getline"
    "close"
)

for op in "${FILE_OPERATIONS[@]}"; do
    if grep -q "$op" "$SOURCE_FILE"; then
        echo "✅ Found file operation: $op"
    else
        echo "❌ FAIL: Missing file operation: $op"
        exit 1
    fi
done

# Test 3: Backup user data
echo "Test 3: Backing up user data..."
CONFIG_DIR="$HOME/.config/mmry"
BACKUP_DIR="/tmp/mmry_backup_$$"

# Create backup directory
mkdir -p "$BACKUP_DIR"

# Backup existing files if they exist
if [ -d "$CONFIG_DIR" ]; then
    echo "Backing up existing MMRY data..."
    cp -r "$CONFIG_DIR" "$BACKUP_DIR/" 2>/dev/null || true
    echo "✅ User data backed up to $BACKUP_DIR"
else
    echo "No existing MMRY data to backup"
fi

# Test 4: Start application and test file creation
echo "Test 4: Testing file creation..."
MMRY_PID=$!

sleep 2

if ! kill -0 $MMRY_PID 2>/dev/null; then
    echo "❌ FAIL: Application failed to start"
    # Restore backup
    if [ -d "$BACKUP_DIR/mmry" ]; then
        rm -rf "$CONFIG_DIR" 2>/dev/null || true
        mv "$BACKUP_DIR/mmry" "$HOME/.config/" 2>/dev/null || true
    fi
    rm -rf "$BACKUP_DIR" 2>/dev/null || true
    exit 1
fi

# Test 5: Check required files are created
echo "Test 5: Verifying required files..."
REQUIRED_FILES=(
    "$CONFIG_DIR/config.json"
    "$CONFIG_DIR/clips.txt"
    "$CONFIG_DIR/bookmarks.txt"
    "$CONFIG_DIR/themes/console.json"
)

for file in "${REQUIRED_FILES[@]}"; do
    if [ -f "$file" ]; then
        echo "✅ File created: $(basename "$file")"
    else
        echo "❌ FAIL: File not created: $(basename "$file")"
        kill $MMRY_PID 2>/dev/null || true
        # Restore backup
        if [ -d "$BACKUP_DIR/mmry" ]; then
            rm -rf "$CONFIG_DIR" 2>/dev/null || true
            mv "$BACKUP_DIR/mmry" "$HOME/.config/" 2>/dev/null || true
        fi
        rm -rf "$BACKUP_DIR" 2>/dev/null || true
        exit 1
    fi
done

# Test 6: Test data persistence
echo "Test 6: Testing data persistence..."

# Add some test clipboard content
echo "Test persistence item 1" | xclip -selection clipboard 2>/dev/null || echo "Test persistence item 1" | pbcopy 2>/dev/null || true
sleep 0.5
echo "Test persistence item 2" | xclip -selection clipboard 2>/dev/null || echo "Test persistence item 2" | pbcopy 2>/dev/null || true
sleep 0.5
echo "Test persistence item 3" | xclip -selection clipboard 2>/dev/null || echo "Test persistence item 3" | pbcopy 2>/dev/null || true
sleep 1

# Kill application to test save
kill $MMRY_PID 2>/dev/null || true
sleep 1

# Restart application
MMRY_PID=$!

sleep 2

if ! kill -0 $MMRY_PID 2>/dev/null; then
    echo "❌ FAIL: Application failed to restart"
    # Restore backup
    if [ -d "$BACKUP_DIR/mmry" ]; then
        rm -rf "$CONFIG_DIR" 2>/dev/null || true
        mv "$BACKUP_DIR/mmry" "$HOME/.config/" 2>/dev/null || true
    fi
    rm -rf "$BACKUP_DIR" 2>/dev/null || true
    exit 1
fi

# Check if data persisted
CLIPS_FILE="$CONFIG_DIR/clips.txt"
if [ -f "$CLIPS_FILE" ]; then
    LINE_COUNT=$(wc -l < "$CLIPS_FILE")
    if [ "$LINE_COUNT" -ge 3 ]; then
        echo "✅ Data persistence working ($LINE_COUNT items saved)"
    else
        echo "❌ FAIL: Data persistence failed (only $LINE_COUNT items)"
        kill $MMRY_PID 2>/dev/null || true
        # Restore backup
        if [ -d "$BACKUP_DIR/mmry" ]; then
            rm -rf "$CONFIG_DIR" 2>/dev/null || true
            mv "$BACKUP_DIR/mmry" "$HOME/.config/" 2>/dev/null || true
        fi
        rm -rf "$BACKUP_DIR" 2>/dev/null || true
        exit 1
    fi
else
    echo "❌ FAIL: Clips file not found after restart"
    kill $MMRY_PID 2>/dev/null || true
    # Restore backup
    if [ -d "$BACKUP_DIR/mmry" ]; then
        rm -rf "$CONFIG_DIR" 2>/dev/null || true
        mv "$BACKUP_DIR/mmry" "$HOME/.config/" 2>/dev/null || true
    fi
    rm -rf "$BACKUP_DIR" 2>/dev/null || true
    exit 1
fi

kill $MMRY_PID 2>/dev/null || true

# Test 7: Check file permissions
echo "Test 7: Verifying file permissions..."
for file in "${REQUIRED_FILES[@]}"; do
    if [ -f "$file" ]; then
        if [ -r "$file" ] && [ -w "$file" ]; then
            echo "✅ File permissions correct: $(basename "$file")"
        else
            echo "❌ FAIL: Incorrect permissions: $(basename "$file")"
            # Restore backup
            if [ -d "$BACKUP_DIR/mmry" ]; then
                rm -rf "$CONFIG_DIR" 2>/dev/null || true
                mv "$BACKUP_DIR/mmry" "$HOME/.config/" 2>/dev/null || true
            fi
            rm -rf "$BACKUP_DIR" 2>/dev/null || true
            exit 1
        fi
    fi
done

# Test 8: Check for error handling in file operations
echo "Test 8: Verifying error handling..."
if grep -q "is_open" "$SOURCE_FILE" && grep -q "good" "$SOURCE_FILE"; then
    echo "✅ File error handling implemented"
else
    echo "⚠️  WARNING: File error handling may not be comprehensive"
fi

# Restore user data
echo "Test 9: Restoring user data..."
if [ -d "$BACKUP_DIR/mmry" ]; then
    # Remove test data
    rm -rf "$CONFIG_DIR" 2>/dev/null || true
    # Restore original data
    mv "$BACKUP_DIR/mmry" "$HOME/.config/" 2>/dev/null || true
    echo "✅ User data restored"
else
    # No backup existed, remove test data
    rm -rf "$CONFIG_DIR" 2>/dev/null || true
    echo "✅ Test data cleaned up"
fi

rm -rf "$BACKUP_DIR" 2>/dev/null || true

echo ""
echo "✅ FILE OPERATIONS TEST PASSED"
exit 0
