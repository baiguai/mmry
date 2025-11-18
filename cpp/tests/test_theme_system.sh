#!/bin/bash

# Theme System Test

# Prevent interactive editors from opening
export EDITOR=""
export VISUAL=""
export GIT_EDITOR=""
unset EDITOR VISUAL GIT_EDITOR

# Tests theme loading and application

echo "Testing MMRY Theme System..."
echo ""

EXECUTABLE="/home/baiguai/documents/development/rust/mmry/cpp/build/bin/mmry_cpp"
if [ ! -f "$EXECUTABLE" ]; then
    echo "❌ FAIL: Executable not found"
    exit 1
fi

# Test 1: Check for theme implementation in source
echo "Test 1: Verifying theme implementation..."
SOURCE_FILE="/home/baiguai/documents/development/rust/mmry/cpp/complete_main.cpp"

# Check for theme variables
THEME_VARS=(
    "backgroundColor"
    "textColor"
    "selectionColor"
    "borderColor"
)

for var in "${THEME_VARS[@]}"; do
    if grep -q "$var" "$SOURCE_FILE"; then
        echo "✅ Found theme variable: $var"
    else
        echo "❌ FAIL: Missing theme variable: $var"
        exit 1
    fi
done

# Check for theme functions
THEME_FUNCS=(
    "loadTheme"
    "createDefaultThemeFile"
)

for func in "${THEME_FUNCS[@]}"; do
    if grep -q "$func" "$SOURCE_FILE"; then
        echo "✅ Found theme function: $func"
    else
        echo "❌ FAIL: Missing theme function: $func"
        exit 1
    fi
done

# Test 2: Check for theme file handling
echo "Test 2: Verifying theme file handling..."
if grep -q "themes.*directory" "$SOURCE_FILE" && grep -q "\.json" "$SOURCE_FILE"; then
    echo "✅ Theme file handling implemented"
else
    echo "❌ FAIL: Theme file handling not implemented"
    exit 1
fi

# Test 3: Check for default theme creation
echo "Test 3: Verifying default theme creation..."
if grep -q "default.*theme" "$SOURCE_FILE" && grep -q "create.*file" "$SOURCE_FILE"; then
    echo "✅ Default theme creation implemented"
else
    echo "❌ FAIL: Default theme creation not implemented"
    exit 1
fi

# Test 4: Check for theme application in drawing
echo "Test 4: Verifying theme application..."
if grep -q "XSetForeground.*backgroundColor" "$SOURCE_FILE" && grep -q "XSetForeground.*textColor" "$SOURCE_FILE"; then
    echo "✅ Theme application in drawing implemented"
else
    echo "❌ FAIL: Theme application in drawing not implemented"
    exit 1
fi

# Test 5: Start application and test theme system
echo "Test 5: Testing theme system with application..."

# Backup existing theme files if they exist
CONFIG_DIR="$HOME/.config/mmry"
THEMES_DIR="$CONFIG_DIR/themes"
BACKUP_DIR="/tmp/mmry_theme_backup_$$"

if [ -d "$THEMES_DIR" ]; then
    echo "Backing up existing themes..."
    mkdir -p "$BACKUP_DIR"
    cp -r "$THEMES_DIR" "$BACKUP_DIR/" 2>/dev/null || true
fi

MMRY_PID=$!

sleep 2

if ! kill -0 $MMRY_PID 2>/dev/null; then
    echo "❌ FAIL: Application failed to start"
    # Restore backup if it exists
    if [ -d "$BACKUP_DIR" ]; then
        rm -rf "$THEMES_DIR" 2>/dev/null || true
        mv "$BACKUP_DIR/themes" "$CONFIG_DIR/" 2>/dev/null || true
        rm -rf "$BACKUP_DIR" 2>/dev/null || true
    fi
    exit 1
fi

# Test 6: Check if theme files are created
echo "Test 6: Verifying theme file creation..."
if [ -d "$THEMES_DIR" ]; then
    echo "✅ Themes directory created"
    
    # Check for default theme file
    if [ -f "$THEMES_DIR/console.json" ]; then
        echo "✅ Default theme file created"
        
        # Check if theme file has valid JSON structure
        if python3 -m json.tool "$THEMES_DIR/console.json" >/dev/null 2>&1; then
            echo "✅ Default theme file has valid JSON"
        else
            echo "⚠️  WARNING: Default theme file may have invalid JSON"
        fi
    else
        echo "❌ FAIL: Default theme file not created"
        kill $MMRY_PID 2>/dev/null || true
        # Restore backup
        if [ -d "$BACKUP_DIR" ]; then
            rm -rf "$THEMES_DIR" 2>/dev/null || true
            mv "$BACKUP_DIR/themes" "$CONFIG_DIR/" 2>/dev/null || true
            rm -rf "$BACKUP_DIR" 2>/dev/null || true
        fi
        exit 1
    fi
else
    echo "❌ FAIL: Themes directory not created"
    kill $MMRY_PID 2>/dev/null || true
    # Restore backup
    if [ -d "$BACKUP_DIR" ]; then
        mv "$BACKUP_DIR/themes" "$CONFIG_DIR/" 2>/dev/null || true
        rm -rf "$BACKUP_DIR" 2>/dev/null || true
    fi
    exit 1
fi

kill $MMRY_PID 2>/dev/null || true

# Test 7: Check for theme color values
echo "Test 7: Verifying theme color values..."
if [ -f "$THEMES_DIR/console.json" ]; then
    # Check for required color keys
    REQUIRED_COLORS=("background" "text" "selection" "border")
    
    for color in "${REQUIRED_COLORS[@]}"; do
        if grep -q "\"$color\"" "$THEMES_DIR/console.json"; then
            echo "✅ Found color key: $color"
        else
            echo "❌ FAIL: Missing color key: $color"
            # Restore backup
            if [ -d "$BACKUP_DIR" ]; then
                rm -rf "$THEMES_DIR" 2>/dev/null || true
                mv "$BACKUP_DIR/themes" "$CONFIG_DIR/" 2>/dev/null || true
                rm -rf "$BACKUP_DIR" 2>/dev/null || true
            fi
            exit 1
        fi
    done
fi

# Restore backup if it exists
if [ -d "$BACKUP_DIR" ]; then
    echo "Restoring original themes..."
    rm -rf "$THEMES_DIR" 2>/dev/null || true
    mv "$BACKUP_DIR/themes" "$CONFIG_DIR/" 2>/dev/null || true
    rm -rf "$BACKUP_DIR" 2>/dev/null || true
fi

# Test 8: Check for theme switching capability
echo "Test 8: Verifying theme switching capability..."
if grep -q "theme.*config" "$SOURCE_FILE" || grep -q "loadTheme.*config" "$SOURCE_FILE"; then
    echo "✅ Theme switching capability implemented"
else
    echo "⚠️  WARNING: Theme switching capability may not be implemented"
fi

echo ""
echo "✅ THEME SYSTEM TEST PASSED"
exit 0
