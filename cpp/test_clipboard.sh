#!/bin/bash

# Test script to simulate clipboard changes
echo "Testing MMRY clipboard monitoring..."
echo "This will write test data to /tmp/mmry_clipboard"

# Test 1: Simple text
echo "Hello World" > /tmp/mmry_clipboard
echo "Test 1: 'Hello World' written to clipboard"

sleep 2

# Test 2: Multi-line text
echo -e "Line 1\nLine 2\nLine 3" > /tmp/mmry_clipboard
echo "Test 2: Multi-line text written"

sleep 2

# Test 3: Long text
echo "This is a very long clipboard entry that should be truncated when displayed in the console window to test the truncation functionality" > /tmp/mmry_clipboard
echo "Test 3: Long text written"

sleep 2

# Test 4: Code snippet
echo -e "function test() {\n    console.log('Hello');\n    return true;\n}" > /tmp/mmry_clipboard
echo "Test 4: Code snippet written"

echo "Testing complete. Check the MMRY window to see the clipboard items."