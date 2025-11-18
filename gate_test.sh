#!/bin/bash

# MMRY Gate Test - Comprehensive Regression Test Suite
# This script runs all regression tests to ensure MMRY functionality
# Usage: ./gate_test.sh

set -e  # Exit on any test failure

echo "=========================================="
echo "MMRY Comprehensive Regression Test Suite"
echo "=========================================="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

TESTS_DIR="$(dirname "$0")/tests"
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Create error log file with timestamp
ERROR_LOG="$(dirname "$0")/$(date +%Y%m%d-%H%M%S)_errors.txt"
echo "Error log will be saved to: $ERROR_LOG"
echo "MMRY Test Run - $(date)" > "$ERROR_LOG"
echo "========================================" >> "$ERROR_LOG"
echo "" >> "$ERROR_LOG"

# Function to run a test and report results
run_test() {
    local test_name="$1"
    local test_script="$2"
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    echo -e "${YELLOW}Running: $test_name${NC}"
    echo "----------------------------------------"
    
    # Cleanup before each test
    pkill -f mmry_cpp 2>/dev/null || true
    sleep 2
    
    # Run test and capture output
    if bash "$test_script" < /dev/null > /dev/null 2>&1; then
        echo -e "${GREEN}✅ PASSED: $test_name${NC}"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        echo -e "${RED}❌ FAILED: $test_name${NC}"
        FAILED_TESTS=$((FAILED_TESTS + 1))
        
        # Capture the error output
        echo "FAILED: $test_name" >> "$ERROR_LOG"
        echo "------------------------" >> "$ERROR_LOG"
        bash "$test_script" < /dev/null >> "$ERROR_LOG" 2>&1 || true
        echo "" >> "$ERROR_LOG"
        echo "" >> "$ERROR_LOG"
    fi
    
    # Cleanup after each test
    pkill -f mmry_cpp 2>/dev/null || true
    sleep 1
    
    echo ""
    echo "=========================================="
    echo ""
}

# Check if tests directory exists
if [ ! -d "$TESTS_DIR" ]; then
    echo -e "${RED}Error: Tests directory not found at $TESTS_DIR${NC}"
    exit 1
fi

# Build the project first
echo "Building MMRY..."
echo "----------------------------------------"
if ./build.sh; then
    echo -e "${GREEN}✅ Build successful${NC}"
else
    echo -e "${RED}❌ Build failed${NC}"
    exit 1
fi
echo ""

# Kill any existing MMRY processes to avoid conflicts
echo "Cleaning up existing MMRY processes..."
pkill -f mmry_cpp 2>/dev/null || true
pkill -f "mmry_cpp" 2>/dev/null || true
sleep 3
echo ""

# Run all regression tests
run_test "Build System Test" "$TESTS_DIR/test_build.sh"
run_test "Basic Functionality Test" "$TESTS_DIR/test_basic_functionality.sh"
run_test "Clipboard Operations Test" "$TESTS_DIR/test_clipboard_operations.sh"
run_test "Navigation Test" "$TESTS_DIR/test_navigation.sh"
run_test "Scrolling Test" "$TESTS_DIR/test_scrolling.sh"
run_test "Bookmark Management Test" "$TESTS_DIR/test_bookmark_management.sh"
run_test "Filter Functionality Test" "$TESTS_DIR/test_filter_functionality.sh"
run_test "Help Dialog Test" "$TESTS_DIR/test_help_dialog.sh"
run_test "Hotkey Test" "$TESTS_DIR/test_hotkeys.sh"
run_test "Theme System Test" "$TESTS_DIR/test_theme_system.sh"
run_test "File Operations Test" "$TESTS_DIR/test_file_operations.sh"
run_test "File Recreation Test" "$TESTS_DIR/test_file_recreation.sh"
run_test "Error Handling Test" "$TESTS_DIR/test_error_handling.sh"

# Final summary
echo "=========================================="
echo "FINAL TEST RESULTS"
echo "=========================================="
echo -e "Total Tests: $TOTAL_TESTS"
echo -e "${GREEN}Passed: $PASSED_TESTS${NC}"
echo -e "${RED}Failed: $FAILED_TESTS${NC}"

if [ $FAILED_TESTS -gt 0 ]; then
    echo ""
    echo -e "${YELLOW}Failed test details saved to: $ERROR_LOG${NC}"
fi

if [ $FAILED_TESTS -eq 0 ]; then
    echo ""
    echo -e "${GREEN}🎉 ALL TESTS PASSED! MMRY is ready for release.${NC}"
    # Remove empty error log if all tests passed
    rm -f "$ERROR_LOG"
    exit 0
else
    echo ""
    echo -e "${RED}💥 SOME TESTS FAILED! Please fix issues before proceeding.${NC}"
    exit 1
fi