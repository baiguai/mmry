#!/bin/bash

# Fix all test files with absolute paths
TESTS_DIR="/home/baiguai/documents/development/rust/mmry/cpp/tests"
EXECUTABLE_PATH="/home/baiguai/documents/development/rust/mmry/cpp/build/bin/mmry_cpp"
SOURCE_FILE="/home/baiguai/documents/development/rust/mmry/cpp/complete_main.cpp"

for test_file in "$TESTS_DIR"/test_*.sh; do
    if [ -f "$test_file" ]; then
        echo "Fixing $(basename "$test_file")..."
        
        # Replace executable path
        sed -i "s|EXECUTABLE=\".*\"|EXECUTABLE=\"$EXECUTABLE_PATH\"|g" "$test_file"
        
        # Replace source file paths in grep commands
        sed -i "s|\.\./complete_main\.cpp|$SOURCE_FILE|g" "$test_file"
        
        # Remove any cd commands that might cause issues
        sed -i '/cd "\$(dirname "\$0")\/\.\."/d' "$test_file"
        
        echo "✅ Fixed $(basename "$test_file")"
    fi
done

echo "All test files fixed with absolute paths!"