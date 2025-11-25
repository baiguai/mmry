#!/bin/bash

# MMRY Process Cleanup Script
# This script helps clean up hung MMRY processes and related clipboard utilities

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check if a process is running
is_process_running() {
    pgrep -f "$1" > /dev/null 2>&1
}

# Function to forcefully kill processes
kill_processes() {
    local process_name="$1"
    local description="$2"
    
    print_status "Checking for $description processes..."
    
    if is_process_running "$process_name"; then
        print_warning "Found running $description processes:"
        pgrep -f "$process_name" | while read pid; do
            echo "  PID: $pid"
            ps -p $pid -o pid,ppid,cmd --no-headers 2>/dev/null | sed 's/^/    /'
        done
        
        print_status "Attempting graceful termination..."
        pkill -TERM -f "$process_name" || true
        sleep 2
        
        if is_process_running "$process_name"; then
            print_warning "Graceful termination failed, using force..."
            pkill -KILL -f "$process_name" || true
            sleep 1
        fi
        
        if ! is_process_running "$process_name"; then
            print_success "Successfully terminated all $description processes"
        else
            print_error "Failed to terminate some $description processes"
            return 1
        fi
    else
        print_success "No $description processes found"
    fi
}

# Function to clean up lock files
cleanup_lock_files() {
    print_status "Cleaning up lock files..."
    
    local lock_files=(
        "/tmp/mmry.lock"
        "/tmp/mmry_test.lock"
        "$HOME/.config/mmry/mmry.lock"
        "/var/lock/mmry.lock"
    )
    
    for lock_file in "${lock_files[@]}"; do
        if [ -f "$lock_file" ]; then
            print_warning "Removing lock file: $lock_file"
            rm -f "$lock_file" || print_error "Failed to remove $lock_file"
        fi
    done
    
    print_success "Lock file cleanup completed"
}

# Function to clean up X11 selections
cleanup_x11_clipboard() {
    print_status "Cleaning up X11 clipboard selections..."
    
    if command -v xclip >/dev/null 2>&1; then
        # Clear primary and clipboard selections
        echo "" | xclip -selection primary 2>/dev/null || true
        echo "" | xclip -selection clipboard 2>/dev/null || true
        print_success "X11 clipboard selections cleared"
    fi
    
    if command -v xsel >/dev/null 2>&1; then
        xsel --clear --primary 2>/dev/null || true
        xsel --clear --clipboard 2>/dev/null || true
        print_success "XSel clipboard cleared"
    fi
}

# Function to reset X11 resource grabs
reset_x11_grabs() {
    print_status "Resetting X11 key grabs..."
    
    if command -v xdotool >/dev/null 2>&1 && xdotool getdisplay >/dev/null 2>&1; then
        # Try to release any stuck key grabs
        xdotool keyup Control Alt Shift Super 2>/dev/null || true
        print_success "X11 key states reset"
    fi
}

# Function to show current status
show_status() {
    print_status "Current process status:"
    
    echo -e "\n${YELLOW}MMRY Processes:${NC}"
    if pgrep -f "mmry" >/dev/null 2>&1; then
        pgrep -f "mmry" | while read pid; do
            echo "  PID: $pid - $(ps -p $pid -o cmd --no-headers 2>/dev/null || echo 'Process not found')"
        done
    else
        echo "  No MMRY processes found"
    fi
    
    echo -e "\n${YELLOW}Clipboard Utilities:${NC}"
    for cmd in xclip xsel; do
        if pgrep -f "$cmd" >/dev/null 2>&1; then
            pgrep -f "$cmd" | while read pid; do
                echo "  $cmd PID: $pid"
            done
        else
            echo "  No $cmd processes found"
        fi
    done
    
    echo -e "\n${YELLOW}Lock Files:${NC}"
    for lock_file in "/tmp/mmry.lock" "/tmp/mmry_test.lock" "$HOME/.config/mmry/mmry.lock"; do
        if [ -f "$lock_file" ]; then
            echo "  Found: $lock_file"
        fi
    done
}

# Function to provide help
show_help() {
    echo "MMRY Process Cleanup Script"
    echo ""
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -h, --help      Show this help message"
    echo "  -s, --status    Show current status only"
    echo "  -f, --force     Force kill all processes (skip graceful termination)"
    echo "  -q, --quiet     Quiet mode (minimal output)"
    echo "  --mmry-only     Only clean MMRY processes"
    echo "  --clipboard-only Only clean clipboard utilities"
    echo ""
    echo "Examples:"
    echo "  $0              # Full cleanup"
    echo "  $0 --status     # Show current status"
    echo "  $0 --force      # Force kill everything"
    echo "  $0 --mmry-only  # Only clean MMRY processes"
}

# Main script logic
main() {
    local force_mode=false
    local quiet_mode=false
    local status_only=false
    local mmry_only=false
    local clipboard_only=false
    
    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -s|--status)
                status_only=true
                shift
                ;;
            -f|--force)
                force_mode=true
                shift
                ;;
            -q|--quiet)
                quiet_mode=true
                shift
                ;;
            --mmry-only)
                mmry_only=true
                shift
                ;;
            --clipboard-only)
                clipboard_only=true
                shift
                ;;
            *)
                print_error "Unknown option: $1"
                show_help
                exit 1
                ;;
        esac
    done
    
    # If quiet mode, redirect output
    if [ "$quiet_mode" = true ]; then
        exec 1>/dev/null
    fi
    
    echo -e "${BLUE}=== MMRY Process Cleanup Script ===${NC}"
    
    # Show status if requested
    if [ "$status_only" = true ]; then
        show_status
        exit 0
    fi
    
    # Perform cleanup
    if [ "$clipboard_only" = false ]; then
        # Clean MMRY processes
        if [ "$force_mode" = true ]; then
            pkill -KILL -f "mmry" || true
        else
            kill_processes "mmry" "MMRY"
        fi
    fi
    
    if [ "$mmry_only" = false ]; then
        # Clean clipboard utilities
        if [ "$force_mode" = true ]; then
            pkill -KILL -f "xclip" || true
            pkill -KILL -f "xsel" || true
        else
            kill_processes "xclip" "xclip"
            kill_processes "xsel" "xsel"
        fi
    fi
    
    # Always clean up lock files and reset X11
    cleanup_lock_files
    cleanup_x11_clipboard
    reset_x11_grabs
    
    # Show final status
    echo -e "\n${BLUE}=== Final Status ===${NC}"
    show_status
    
    print_success "Cleanup completed!"
    
    if [ "$quiet_mode" = false ]; then
        echo -e "\n${YELLOW}Tips:${NC}"
        echo "- If MMRY still won't start, try: $0 --force"
        echo "- To check status: $0 --status"
        echo "- If issues persist, try restarting your X11 session"
    fi
}

# Run main function with all arguments
main "$@"