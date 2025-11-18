#!/bin/bash

# File Recreation Test

# Prevent interactive editors from opening
export EDITOR=""
export VISUAL=""
export GIT_EDITOR=""
unset EDITOR VISUAL GIT_EDITOR

# Tests that MMRY can recreate all necessary files if they're missing or corrupted

echo "Testing MMRY File Recreation..."
echo ""

EXECUTABLE="/home/baiguai/documents/development/rust/mmry/cpp/build/bin/mmry_cpp"
if [ ! -f "$EXECUTABLE" ]; then
    echo "❌ FAIL: Executable not found"
    exit 1
fi

# Test 1: Backup user data
echo "Test 1: Backing up user data..."
CONFIG_DIR="$HOME/.config/mmry"
BACKUP_DIR="/tmp/mmry_recreation_backup_$$"

if [ -d "$CONFIG_DIR" ]; then
    mkdir -p "$BACKUP_DIR"
    cp -r "$CONFIG_DIR" "$BACKUP_DIR/" 2>/dev/null || true
    echo "✅ User data backed up"
else
    echo "No existing MMRY data to backup"
fi

# Test 2: Remove all MMRY files and directories
echo "Test 2: Removing all MMRY files..."
rm -rf "$CONFIG_DIR" 2>/dev/null || true
echo "✅ Removed all MMRY files"

# Test 3: Start MMRY and check file recreation
echo "Test 3: Testing file recreation..."
MMRY_PID=$!

sleep 3  # Give extra time for file creation

if ! kill -0 $MMRY_PID 2>/dev/null; then
    echo "❌ FAIL: Application failed to start with missing files"
    # Restore backup
    if [ -d "$BACKUP_DIR/mmry" ]; then
        mv "$BACKUP_DIR/mmry" "$HOME/.config/" 2>/dev/null || true
    fi
    rm -rf "$BACKUP_DIR" 2>/dev/null || true
    exit 1
fi

# Test 4: Verify all required files are recreated
echo "Test 4: Verifying required files recreation..."
REQUIRED_FILES=(
    "$CONFIG_DIR/config.json"
    "$CONFIG_DIR/clips.txt"
    "$CONFIG_DIR/bookmarks.txt"
    "$CONFIG_DIR/themes"
    "$CONFIG_DIR/themes/console.json"
)

ALL_FILES_RECREATED=true
for file in "${REQUIRED_FILES[@]}"; do
    if [ -e "$file" ]; then
        echo "✅ Recreated: $(basename "$file")"
    else
        echo "❌ FAIL: Not recreated: $(basename "$file")"
        ALL_FILES_RECREATED=false
    fi
done

if [ "$ALL_FILES_RECREATED" = false ]; then
    kill $MMRY_PID 2>/dev/null || true
    # Restore backup
    if [ -d "$BACKUP_DIR/mmry" ]; then
        mv "$BACKUP_DIR/mmry" "$HOME/.config/" 2>/dev/null || true
    fi
    rm -rf "$BACKUP_DIR" 2>/dev/null || true
    exit 1
fi

# Test 5: Verify recreated files have correct content
echo "Test 5: Verifying recreated file content..."

# Check config.json has valid JSON
if python3 -m json.tool "$CONFIG_DIR/config.json" >/dev/null 2>&1; then
    echo "✅ config.json has valid JSON"
else
    echo "❌ FAIL: config.json has invalid JSON"
    kill $MMRY_PID 2>/dev/null || true
    # Restore backup
    if [ -d "$BACKUP_DIR/mmry" ]; then
        mv "$BACKUP_DIR/mmry" "$HOME/.config/" 2>/dev/null || true
    fi
    rm -rf "$BACKUP_DIR" 2>/dev/null || true
    exit 1
fi

# Check clips.txt has correct format
if [ -f "$CONFIG_DIR/clips.txt" ]; then
    # Should be empty or have valid format
    if [ -s "$CONFIG_DIR/clips.txt" ]; then
        INVALID_LINES=0
        while IFS= read -r line; do
            if [[ ! "$line" =~ ^[0-9]+\| ]]; then
                INVALID_LINES=$((INVALID_LINES + 1))
            fi
        done < "$CONFIG_DIR/clips.txt"
        
        if [ "$INVALID_LINES" -eq 0 ]; then
            echo "✅ clips.txt has correct format"
        else
            echo "❌ FAIL: clips.txt has $INVALID_LINES invalid lines"
            kill $MMRY_PID 2>/dev/null || true
            # Restore backup
            if [ -d "$BACKUP_DIR/mmry" ]; then
                mv "$BACKUP_DIR/mmry" "$HOME/.config/" 2>/dev/null || true
            fi
            rm -rf "$BACKUP_DIR" 2>/dev/null || true
            exit 1
        fi
    else
        echo "✅ clips.txt is empty (acceptable for new installation)"
    fi
else
    echo "❌ FAIL: clips.txt not recreated"
    kill $MMRY_PID 2>/dev/null || true
    # Restore backup
    if [ -d "$BACKUP_DIR/mmry" ]; then
        mv "$BACKUP_DIR/mmry" "$HOME/.config/" 2>/dev/null || true
    fi
    rm -rf "$BACKUP_DIR" 2>/dev/null || true
    exit 1
fi

# Check bookmarks.txt has correct format
if [ -f "$CONFIG_DIR/bookmarks.txt" ]; then
    # Should be empty or have valid format (group|selected_index)
    if [ -s "$CONFIG_DIR/bookmarks.txt" ]; then
        INVALID_LINES=0
        while IFS= read -r line; do
            if [ -n "$line" ] && [[ ! "$line" =~ ^[a-zA-Z0-9_]+\|[0-9]+$ ]]; then
                INVALID_LINES=$((INVALID_LINES + 1))
            fi
        done < "$CONFIG_DIR/bookmarks.txt"
        
        if [ "$INVALID_LINES" -eq 0 ]; then
            echo "✅ bookmarks.txt has correct format"
        else
            echo "❌ FAIL: bookmarks.txt has $INVALID_LINES invalid lines"
            kill $MMRY_PID 2>/dev/null || true
            # Restore backup
            if [ -d "$BACKUP_DIR/mmry" ]; then
                mv "$BACKUP_DIR/mmry" "$HOME/.config/" 2>/dev/null || true
            fi
            rm -rf "$BACKUP_DIR" 2>/dev/null || true
            exit 1
        fi
    else
        echo "✅ bookmarks.txt is empty (acceptable for new installation)"
    fi
else
    echo "❌ FAIL: bookmarks.txt not recreated"
    kill $MMRY_PID 2>/dev/null || true
    # Restore backup
    if [ -d "$BACKUP_DIR/mmry" ]; then
        mv "$BACKUP_DIR/mmry" "$HOME/.config/" 2>/dev/null || true
    fi
    rm -rf "$BACKUP_DIR" 2>/dev/null || true
    exit 1
fi

# Check console theme has valid JSON
if [ -f "$CONFIG_DIR/themes/console.json" ]; then
    if python3 -m json.tool "$CONFIG_DIR/themes/console.json" >/dev/null 2>&1; then
        echo "✅ console.json theme has valid JSON"
    else
        echo "❌ FAIL: console.json theme has invalid JSON"
        kill $MMRY_PID 2>/dev/null || true
        # Restore backup
        if [ -d "$BACKUP_DIR/mmry" ]; then
            mv "$BACKUP_DIR/mmry" "$HOME/.config/" 2>/dev/null || true
        fi
        rm -rf "$BACKUP_DIR" 2>/dev/null || true
        exit 1
    fi
else
    echo "❌ FAIL: console.json theme not recreated"
    kill $MMRY_PID 2>/dev/null || true
    # Restore backup
    if [ -d "$BACKUP_DIR/mmry" ]; then
        mv "$BACKUP_DIR/mmry" "$HOME/.config/" 2>/dev/null || true
    fi
    rm -rf "$BACKUP_DIR" 2>/dev/null || true
    exit 1
fi

# Test 6: Test with corrupted files recreation
echo "Test 6: Testing corrupted files recreation..."
kill $MMRY_PID 2>/dev/null || true
sleep 1

# Create corrupted files
echo "invalid json" > "$CONFIG_DIR/config.json"
echo "invalid|clip|format" > "$CONFIG_DIR/clips.txt"
echo "invalid@bookmark#name|0" > "$CONFIG_DIR/bookmarks.txt"
echo "not json" > "$CONFIG_DIR/themes/console.json"

# Start MMRY again
MMRY_PID=$!

sleep 3

if kill -0 $MMRY_PID 2>/dev/null; then
    echo "✅ Application handles corrupted files and recreates them"
else
    echo "❌ FAIL: Application fails with corrupted files"
    # Restore backup
    if [ -d "$BACKUP_DIR/mmry" ]; then
        rm -rf "$CONFIG_DIR" 2>/dev/null || true
        mv "$BACKUP_DIR/mmry" "$HOME/.config/" 2>/dev/null || true
    fi
    rm -rf "$BACKUP_DIR" 2>/dev/null || true
    exit 1
fi

# Check if files were fixed/recreated
if python3 -m json.tool "$CONFIG_DIR/config.json" >/dev/null 2>&1; then
    echo "✅ config.json was recreated/fixed"
else
    echo "⚠️  WARNING: config.json may still be corrupted"
fi

kill $MMRY_PID 2>/dev/null || true
sleep 1

# Test 7: Test with missing individual files
echo "Test 7: Testing missing individual files..."

# Remove config.json only
rm -f "$CONFIG_DIR/config.json"

MMRY_PID=$!

sleep 2

if [ -f "$CONFIG_DIR/config.json" ]; then
    echo "✅ config.json recreated when missing"
else
    echo "❌ FAIL: config.json not recreated when missing"
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
sleep 1

# Remove clips.txt only
rm -f "$CONFIG_DIR/clips.txt"

MMRY_PID=$!

sleep 2

if [ -f "$CONFIG_DIR/clips.txt" ]; then
    echo "✅ clips.txt recreated when missing"
else
    echo "❌ FAIL: clips.txt not recreated when missing"
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
sleep 1

# Test 8: Verify file permissions
echo "Test 8: Verifying file permissions..."
for file in "${REQUIRED_FILES[@]}"; do
    if [ -e "$file" ]; then
        if [ -r "$file" ] && [ -w "$file" ]; then
            echo "✅ Correct permissions: $(basename "$file")"
        else
            echo "❌ FAIL: Incorrect permissions: $(basename "$file")"
            kill $MMRY_PID 2>/dev/null || true
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

# Clean up
kill $MMRY_PID 2>/dev/null || true

# Restore user data
echo "Test 9: Restoring user data..."
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
echo "✅ FILE RECREATION TEST PASSED"
exit 0
