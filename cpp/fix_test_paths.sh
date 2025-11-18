#!/bin/bash

# Fix paths in all test files
TESTS_DIR="$(dirname "$0")/tests"

for test_file in "$TESTS_DIR"/test_*.sh; do
    if [ -f "$test_file" ]; then
        echo "Fixing paths in $(basename "$test_file")..."
        
        # Fix executable path
        sed -i 's|EXECUTABLE="\./build/bin/mmry_cpp"|EXECUTABLE="$(dirname "$0")/../build/bin/mmry_cpp"|g' "$test_file"
        
        # Fix executable calls to change directory first
        sed -i 's|"$EXECUTABLE" &|cd "$(dirname "$0")/.." && "$EXECUTABLE" &|g' "$test_file"
        sed -i 's|"$EXECUTABLE"$|cd "$(dirname "$0")/.." && "$EXECUTABLE"|g' "$test_file"
        sed -i 's|timeout.*"$EXECUTABLE"|cd "$(dirname "$0")/.." & timeout* "$EXECUTABLE"|g' "$test_file"
        
        echo "✅ Fixed $(basename "$test_file")"
    fi
done

echo "All test files fixed!"