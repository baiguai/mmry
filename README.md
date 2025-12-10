# MMRY Clipboard Manager

A fast, lightweight clipboard manager written in C++ with X11 support.

## Building

### Option 1: Using Make (Recommended)
```bash
# Build the project
make

# Or build and run
make run

# Clean build artifacts
make clean

# Install to system
make install

# Run tests
make test

# See all options
make help
```

### Option 2: Using Build Script
```bash
# Build using the script
./build.sh

# Build Windows executable
./build-windows.sh

# Build and run
./run.sh
```

### Option 3: Using CMake directly
```bash
mkdir build && cd build
cmake ..
make
```

## Portability

This project uses **relative paths** throughout, making it fully portable:

- All test scripts use `$(dirname "$0")/..` to reference project root
- No hardcoded absolute paths in build system
- CMake configuration is location-independent
- Makefile works from any directory containing the project

Simply copy the entire project directory to any location and it will build and run without modifications.

## Usage

Start the application:
```bash
./build/bin/mmry_cpp
# or
make run
# or (if installed)
mmry_cpp
```

### Hotkeys
- `Ctrl+Alt+C` - Show/hide clipboard window
- `?` - Show help dialog
- `Shift+M` - Bookmark management
- `/` - Filter clipboard items
- `Escape` - Hide window
- `Shift+Q` - Quit application

### Configuration
Configuration files are stored in `~/.config/mmry/`

## Testing

Run the test suite:
```bash
make test
# or
./tests/test_build.sh
```

Individual tests can be run:
```bash
./tests/test_basic_functionality.sh
./tests/test_clipboard_operations.sh
# etc.
```

## Dependencies

- CMake 3.16+
- C++17 compiler (GCC, Clang)
- X11 libraries (Linux)
- Cocoa/CoreFoundation (macOS)
- User32/Kernel32/GDI32 (Windows)
