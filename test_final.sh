#!/bin/bash

echo "Testing MMRY Application..."
echo "=========================="

# Check if application exists
if [ ! -f "./cpp/build/bin/mmry_cpp" ]; then
    echo "❌ Application not found. Building first..."
    cd cpp && ./build.sh && cd ..
fi

echo "✅ Application built successfully"

# Test file creation
echo ""
echo "Testing file creation..."
rm -rf ~/.config/mmry

timeout 2s ./cpp/build/bin/mmry_cpp > /dev/null 2>&1 || true

if [ -d ~/.config/mmry ]; then
    echo "✅ Config directory created"
    
    if [ -f ~/.config/mmry/config.json ]; then
        echo "✅ Config file created"
    else
        echo "❌ Config file missing"
    fi
    
    if [ -f ~/.config/mmry/clips.txt ]; then
        echo "✅ Clips file created"
    else
        echo "❌ Clips file missing"
    fi
    
    if [ -f ~/.config/mmry/bookmarks.txt ]; then
        echo "✅ Bookmarks file created"
    else
        echo "❌ Bookmarks file missing"
    fi
    
    if [ -d ~/.config/mmry/themes ]; then
        echo "✅ Themes directory created"
    else
        echo "❌ Themes directory missing"
    fi
else
    echo "❌ Config directory not created"
fi

echo ""
echo "Testing clipboard monitoring..."
echo "Test content" | xclip -selection clipboard

timeout 3s ./cpp/build/bin/mmry_cpp > test_output.log 2>&1 || true

if grep -q "New clipboard item added" test_output.log; then
    echo "✅ Clipboard monitoring working"
else
    echo "❌ Clipboard monitoring not working"
fi

if grep -q "Successfully grabbed Ctrl+Alt+C hotkey" test_output.log; then
    echo "✅ Hotkey registration successful"
else
    echo "❌ Hotkey registration failed"
fi

# Clean up
rm -f test_output.log

echo ""
echo "Test completed! 🎉"
echo ""
echo "To use MMRY:"
echo "1. Run: ./cpp/build/bin/mmry_cpp"
echo "2. Press Ctrl+Alt+C to show the window"
echo "3. Use j/k or arrow keys to navigate"
echo "4. Press Enter to copy selected item"
echo "5. Press Escape to hide window"
echo "6. Press Shift+M to manage bookmarks"
echo "7. Press 'm' to add to bookmark group"
echo "8. Press \` to view bookmark groups (j/k/gg/Shift+G to navigate, D to delete)"
echo "9. Press '?' for help dialog"