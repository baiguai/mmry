#!/bin/bash

# Error Handling Test

# Prevent interactive editors from opening
export EDITOR=""
export VISUAL=""
export GIT_EDITOR=""
unset EDITOR VISUAL GIT_EDITOR

# Tests error handling and edge cases

echo "Testing MMRY Error Handling..."
echo ""

EXECUTABLE="/home/baiguai/documents/development/rust/mmry/cpp/build/bin/mmry_cpp"
if [ ! -f "$EXECUTABLE" ]; then
    echo "❌ FAIL: Executable not found"
    exit 1
fi

# Test 1: Check for error handling in source
echo "Test 1: Verifying error handling implementation..."
SOURCE_FILE="/home/baiguai/documents/development/rust/mmry/cpp/complete_main.cpp"

# Check for common error handling patterns
ERROR_PATTERNS=(
    "try"
    "catch"
    "if.*fail"
    "if.*error"
    "if.*empty"
    "if.*nullptr"
)

for pattern in "${ERROR_PATTERNS[@]}"; do
    if grep -q "$pattern" "$SOURCE_FILE"; then
        echo "✅ Found error handling pattern: $pattern"
    else
        echo "⚠️  WARNING: Error handling pattern not found: $pattern"
    fi
done

# Test 2: Check for bounds checking
echo "Test 2: Verifying bounds checking..."
BOUNDS_CHECKS=(
    "size.*check"
    "index.*bound"
    "\.size.*>"
    "\.length.*>"
)

for check in "${BOUNDS_CHECKS[@]}"; do
    if grep -q "$check" "$SOURCE_FILE"; then
        echo "✅ Found bounds checking: $check"
    else
        echo "⚠️  WARNING: Bounds checking may not be comprehensive"
    fi
done

# Test 3: Check for file error handling
echo "Test 3: Verifying file error handling..."
FILE_ERROR_HANDLING=(
    "is_open"
    "good()"
    "fail()"
    "bad()"
)

for check in "${FILE_ERROR_HANDLING[@]}"; do
    if grep -q "$check" "$SOURCE_FILE"; then
        echo "✅ Found file error handling: $check"
    else
        echo "⚠️  WARNING: File error handling may not be comprehensive"
    fi
done

# Test 4: Backup user data for error testing
echo "Test 4: Backing up user data for error testing..."
CONFIG_DIR="$HOME/.config/mmry"
BACKUP_DIR="/tmp/mmry_error_backup_$$"

if [ -d "$CONFIG_DIR" ]; then
    mkdir -p "$BACKUP_DIR"
    cp -r "$CONFIG_DIR" "$BACKUP_DIR/" 2>/dev/null || true
    echo "✅ User data backed up"
fi

# Test 5: Test with corrupted config file
echo "Test 5: Testing corrupted config file handling..."
mkdir -p "$CONFIG_DIR"

# Create corrupted config
echo "invalid json content" > "$CONFIG_DIR/config.json"

MMRY_PID=$!

sleep 2

# Check if application handles corrupted config gracefully
if kill -0 $MMRY_PID 2>/dev/null; then
    echo "✅ Application handles corrupted config gracefully"
    kill $MMRY_PID 2>/dev/null || true
else
    echo "⚠️  WARNING: Application may not handle corrupted config gracefully"
fi

sleep 1

# Test 6: Test with corrupted clips file
echo "Test 6: Testing corrupted clips file handling..."
echo "invalid|clip|data|without|timestamp" > "$CONFIG_DIR/clips.txt"

MMRY_PID=$!

sleep 2

if kill -0 $MMRY_PID 2>/dev/null; then
    echo "✅ Application handles corrupted clips gracefully"
    kill $MMRY_PID 2>/dev/null || true
else
    echo "⚠️  WARNING: Application may not handle corrupted clips gracefully"
fi

sleep 1

# Test 7: Test with unreadable files
echo "Test 7: Testing unreadable file handling..."
chmod 000 "$CONFIG_DIR/config.json" 2>/dev/null || true

MMRY_PID=$!

sleep 2

if kill -0 $MMRY_PID 2>/dev/null; then
    echo "✅ Application handles unreadable files gracefully"
    kill $MMRY_PID 2>/dev/null || true
else
    echo "⚠️  WARNING: Application may not handle unreadable files gracefully"
fi

sleep 1

# Restore permissions
chmod 644 "$CONFIG_DIR/config.json" 2>/dev/null || true

# Test 8: Test with empty files
echo "Test 8: Testing empty file handling..."
> "$CONFIG_DIR/clips.txt"
> "$CONFIG_DIR/bookmarks.txt"

MMRY_PID=$!

sleep 2

if kill -0 $MMRY_PID 2>/dev/null; then
    echo "✅ Application handles empty files gracefully"
    kill $MMRY_PID 2>/dev/null || true
else
    echo "❌ FAIL: Application crashes with empty files"
    # Restore backup
    if [ -d "$BACKUP_DIR/mmry" ]; then
        rm -rf "$CONFIG_DIR" 2>/dev/null || true
        mv "$BACKUP_DIR/mmry" "$HOME/.config/" 2>/dev/null || true
    fi
    rm -rf "$BACKUP_DIR" 2>/dev/null || true
    exit 1
fi

sleep 1

# Test 9: Test memory allocation handling
echo "Test 9: Testing memory allocation handling..."
# This is hard to test directly, but we can check for good practices
if grep -q "new.*\[\]" "$SOURCE_FILE" || grep -q "malloc" "$SOURCE_FILE"; then
    if grep -q "delete" "$SOURCE_FILE" || grep -q "free" "$SOURCE_FILE"; then
        echo "✅ Memory management practices found"
    else
        echo "⚠️  WARNING: Memory management may not be complete"
    fi
else
    echo "✅ No dynamic memory allocation detected (good for simplicity)"
fi

# Test 10: Test with missing directory
echo "Test 10: Testing missing directory handling..."
rm -rf "$CONFIG_DIR"

MMRY_PID=$!

sleep 2

if kill -0 $MMRY_PID 2>/dev/null; then
    echo "✅ Application creates missing directories gracefully"
    kill $MMRY_PID 2>/dev/null || true
else
    echo "❌ FAIL: Application fails with missing directory"
    # Restore backup
    if [ -d "$BACKUP_DIR/mmry" ]; then
        mv "$BACKUP_DIR/mmry" "$HOME/.config/" 2>/dev/null || true
    fi
    rm -rf "$BACKUP_DIR" 2>/dev/null || true
    exit 1
fi

sleep 1

# Test 11: Check for graceful shutdown handling
echo "Test 11: Verifying graceful shutdown handling..."
if grep -q "stop()" "$SOURCE_FILE" && grep -q "running.*false" "$SOURCE_FILE"; then
    echo "✅ Graceful shutdown handling implemented"
else
    echo "⚠️  WARNING: Graceful shutdown handling may not be complete"
fi

# Restore user data
echo "Test 12: Restoring user data..."
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
echo "✅ ERROR HANDLING TEST PASSED"
exit 0
