# MMRY Clipboard Manager Makefile
# This is a simple wrapper around CMake for convenience

# Default target
all: build

# Build the project
build:
	@if [ ! -d "build" ]; then \
		echo "Creating build directory..."; \
		mkdir build; \
	fi
	@cd build && \
	if [ ! -f "Makefile" ]; then \
		echo "Configuring with CMake..."; \
		cmake ..; \
	fi
	@echo "Building..."
	@cd build && make
	@echo "✅ Build complete! Executable: build/bin/mmry_cpp"

# Clean build artifacts
clean:
	@echo "Cleaning build directory..."
	@rm -rf build
	@echo "✅ Clean complete!"

# Install (copy to system location)
install: build
	@echo "Installing mmry_cpp to /usr/local/bin..."
	@sudo cp build/bin/mmry_cpp /usr/local/bin/
	@echo "✅ Installation complete!"

# Uninstall
uninstall:
	@echo "Removing mmry_cpp from /usr/local/bin..."
	@sudo rm -f /usr/local/bin/mmry_cpp
	@echo "✅ Uninstallation complete!"

# Run the application
run: build
	@echo "Starting MMRY..."
	@./build/bin/mmry_cpp

# Test the build
test: build
	@echo "Running build test..."
	@./tests/test_build.sh

# Help target
help:
	@echo "MMRY Clipboard Manager Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  all/build    - Build the project (default)"
	@echo "  clean        - Remove build artifacts"
	@echo "  install      - Install to /usr/local/bin"
	@echo "  uninstall    - Remove from /usr/local/bin"
	@echo "  run          - Build and run the application"
	@echo "  test         - Run build tests"
	@echo "  help         - Show this help message"

.PHONY: all build clean install uninstall run test help