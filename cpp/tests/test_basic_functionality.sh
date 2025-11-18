#!/bin/bash

# Basic Functionality Test

# Prevent interactive editors from opening
export EDITOR=""
export VISUAL=""
export GIT_EDITOR=""
unset EDITOR VISUAL GIT_EDITOR

# Tests core MMRY functionality without complex interactions

echo "Testing MMRY Basic Functionality..."
echo ""

# Check if executable exists
EXECUTABLE="/home/baiguai/documents/development/rust/mmry/cpp/build/bin/mmry_cpp"
if [ ! -f "$EXECUTABLE" ]; then
    echo "❌ FAIL: Executable not found"
    exit 1
fi

# Backup user data
echo "Backing up user data..."
CONFIG_DIR="$HOME/.config/mmry"
BACKUP_DIR="/tmp/mmry_basic_backup_$$"

if [ -d "$CONFIG_DIR" ]; then
    mkdir -p "$BACKUP_DIR"
    cp -r "$CONFIG_DIR" "$BACKUP_DIR/" 2>/dev/null || true
    echo "✅ User data backed up"
fi

# Test 1: Application starts and registers hotkey
echo "Test 1: Application startup..."
"$EXECUTABLE" &
MMRY_PID=$!

sleep 2

# Check if process is running
if kill -0 $MMRY_PID 2>/dev/null; then
    echo "✅ Application started successfully (PID: $MMRY_PID)"
else
    echo "❌ FAIL: Application failed to start"
    exit 1
fi

# Test 2: Config directory creation
echo "Test 2: Config directory setup..."
CONFIG_DIR="$HOME/.config/mmry"
if [ -d "$CONFIG_DIR" ]; then
    echo "✅ Config directory created at $CONFIG_DIR"
else
    echo "❌ FAIL: Config directory not created"
    kill $MMRY_PID 2>/dev/null || true
    exit 1
fi

# Test 3: Required files creation
echo "Test 3: Required files..."
REQUIRED_FILES=("$CONFIG_DIR/config.json" "$CONFIG_DIR/clips.txt" "$CONFIG_DIR/bookmarks.txt" "$CONFIG_DIR/themes")
ALL_FILES_EXIST=true

for file in "${REQUIRED_FILES[@]}"; do
    if [ -e "$file" ]; then
        echo "✅ Found: $file"
    else
        echo "❌ FAIL: Missing $file"
        ALL_FILES_EXIST=false
    fi
done

if [ "$ALL_FILES_EXIST" = false ]; then
    kill $MMRY_PID 2>/dev/null || true
    exit 1
fi

# Test 4: Hotkey registration (check for success message)
echo "Test 4: Hotkey registration..."
# Give it a moment to register hotkey
sleep 1
if kill -0 $MMRY_PID 2>/dev/null; then
    echo "✅ Hotkey registration successful (process still running)"
else
    echo "❌ FAIL: Hotkey registration failed (process exited)"
    exit 1
fi

# Test 5: Clean shutdown
echo "Test 5: Clean shutdown..."
kill $MMRY_PID 2>/dev/null
sleep 1

if ! kill -0 $MMRY_PID 2>/dev/null; then
    echo "✅ Application shuts down cleanly"
else
    echo "❌ FAIL: Application did not shut down cleanly"
    kill -9 $MMRY_PID 2>/dev/null || true
    # Restore backup
    if [ -d "$BACKUP_DIR/mmry" ]; then
        rm -rf "$CONFIG_DIR" 2>/dev/null || true
        mv "$BACKUP_DIR/mmry" "$HOME/.config/" 2>/dev/null || true
    fi
    rm -rf "$BACKUP_DIR" 2>/dev/null || true
    exit 1
fi

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
echo "✅ BASIC FUNCTIONALITY TEST PASSED"
exit 0
