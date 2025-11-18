#!/bin/bash

echo "=== Encryption Test ==="
echo

# Remove old data to start fresh
echo "Removing old data..."
rm -f ~/.config/mmry/config.json ~/.config/mmry/clips.txt

echo "Starting MMRY..."
./mmry.sh &
MMRY_PID=$!
sleep 2

echo "Adding test content..."
echo "Secret message 1" | xclip -selection clipboard
sleep 1
echo "Secret message 2" | xclip -selection clipboard
sleep 1

echo "Checking encrypted clips file..."
echo "Content of clips.txt (should be encrypted):"
cat ~/.config/mmry/clips.txt

echo
echo "Config file (should have encryption_key):"
cat ~/.config/mmry/config.json

echo
echo "If content is encrypted and config has key, encryption is working!"
echo
echo "Press Ctrl+C to stop test"
echo "Or run: kill $MMRY_PID"

trap "echo 'Stopping MMRY...'; kill $MMRY_PID; exit" INT
wait $MMRY_PID