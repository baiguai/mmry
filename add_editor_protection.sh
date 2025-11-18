#!/bin/bash

# Add editor protection to all test files
TESTS_DIR="/home/baiguai/documents/development/rust/mmry/cpp/tests"

for test_file in "$TESTS_DIR"/test_*.sh; do
    if [ -f "$test_file" ]; then
        echo "Adding editor protection to $(basename "$test_file")..."
        
        # Check if protection already exists
        if ! grep -q "Prevent interactive editors" "$test_file"; then
            # Create temporary file with protection added
            temp_file=$(mktemp)
            
            # Add protection after the shebang and comments
            awk '
            BEGIN { skip = 0 }
            /^#!/ { print; next }
            /^# Bookmark/ || /^# Basic/ || /^# Clipboard/ || /^# Build/ || /^# Navigation/ || /^# Scrolling/ || /^# Filter/ || /^# Help/ || /^# Hotkey/ || /^# Theme/ || /^# File/ || /^# Error/ {
                print
                print ""
                print "# Prevent interactive editors from opening"
                print "export EDITOR=\"\""
                print "export VISUAL=\"\""
                print "export GIT_EDITOR=\"\""
                print "unset EDITOR VISUAL GIT_EDITOR"
                print ""
                skip = 1
                next
            }
            skip == 1 && /^echo/ { skip = 0; print; next }
            { print }
            ' "$test_file" > "$temp_file"
            
            # Replace original
            mv "$temp_file" "$test_file"
            echo "✅ Added protection to $(basename "$test_file")"
        else
            echo "⚠️  Protection already exists in $(basename "$test_file")"
        fi
    fi
done

echo "Editor protection added to all test files!"