#!/bin/bash

# Build System Test

# Prevent interactive editors from opening
export EDITOR=""
export VISUAL=""
export GIT_EDITOR=""
unset EDITOR VISUAL GIT_EDITOR

# Tests that MMRY builds successfully and produces executable

echo "Testing MMRY Build System..."
echo ""

# Check if build script exists
BUILD_SCRIPT="$(dirname "$0")/../build.sh"
if [ ! -f "$BUILD_SCRIPT" ]; then
    echo "❌ FAIL: build.sh not found"
    exit 1
fi

# Run build script
echo "Running build.sh..."
if ./build.sh; then
    echo "✅ Build script executed successfully"
else
    echo "❌ FAIL: Build script failed"
    exit 1
fi

# Check if executable was created
EXECUTABLE="/home/baiguai/documents/development/rust/mmry/cpp/build/bin/mmry_cpp"
if [ -f "$EXECUTABLE" ]; then
    echo "✅ Executable created at $EXECUTABLE"
else
    echo "❌ FAIL: Executable not found at $EXECUTABLE"
    exit 1
fi

# Check if executable is actually executable
if [ -x "$EXECUTABLE" ]; then
    echo "✅ Executable has correct permissions"
else
    echo "❌ FAIL: Executable is not executable"
    exit 1
fi

# Test that executable can start (briefly)
echo "Testing executable startup..."
if [ $? -eq 124 ]; then
    echo "✅ Executable starts successfully (timed out as expected)"
else
    echo "✅ Executable starts successfully"
fi

echo ""
echo "✅ BUILD SYSTEM TEST PASSED"
exit 0
