#!/bin/bash

# Fix the broken test files properly
TESTS_DIR="$(dirname "$0")/tests"

# First, restore from backup if available
if [ -f "/tmp/test_backup.tar.gz" ]; then
    cd "$TESTS_DIR"
    tar -xzf /tmp/test_backup.tar.gz
    echo "Restored test files from backup"
else
    echo "No backup found, creating backup first..."
    cd "$TESTS_DIR"
    tar -czf /tmp/test_backup.tar.gz test_*.sh
fi

# Now fix each file properly
for test_file in "$TESTS_DIR"/test_*.sh; do
    if [ -f "$test_file" ]; then
        echo "Properly fixing $(basename "$test_file")..."
        
        # Create a temporary file with the corrected content
        temp_file=$(mktemp)
        
        # Read the file and fix the lines properly
        while IFS= read -r line; do
            # Fix executable path
            if [[ "$line" =~ ^EXECUTABLE=\"\./build/bin/mmry_cpp\" ]]; then
                echo 'EXECUTABLE="$(dirname "$0")/../build/bin/mmry_cpp"'
            # Fix executable calls - simple pattern matching
            elif [[ "$line" =~ ^[[:space:]]*\"\$EXECUTABLE\"[[:space:]]*\&[[:space:]]*$ ]]; then
                echo '        cd "$(dirname "$0")/.."'
                echo '        "$EXECUTABLE" &'
            elif [[ "$line" =~ ^[[:space:]]*timeout.*\"\$EXECUTABLE\" ]]; then
                echo '        cd "$(dirname "$0")/.."'
                echo "        $line"
            elif [[ "$line" =~ ^[[:space:]]*\"\$EXECUTABLE\"[[:space:]]*$ ]]; then
                echo '        cd "$(dirname "$0")/.."'
                echo '        "$EXECUTABLE"'
            # Fix grep commands for source file
            elif [[ "$line" =~ grep[[:space:]]+.*\.\./complete_main\.cpp ]]; then
                echo "${line/../complete_main.cpp/$(dirname "$0")/../complete_main.cpp}"
            else
                echo "$line"
            fi
        done < "$test_file" > "$temp_file"
        
        # Replace the original
        mv "$temp_file" "$test_file"
        echo "✅ Fixed $(basename "$test_file")"
    fi
done

echo "All test files properly fixed!"