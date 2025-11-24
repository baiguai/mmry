cmake_minimum_required(VERSION 3.16)
project(mmry VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform-specific libraries
if(UNIX AND NOT APPLE)
    # Linux/X11
    find_package(X11 REQUIRED)
    set(PLATFORM_LIBS ${X11_LIBRARIES})
elseif(APPLE)
    # macOS
    find_library(COCOA Cocoa)
    find_library(COREFOUNDATION CoreFoundation)
    set(PLATFORM_LIBS ${COCOA} ${COREFOUNDATION})
elseif(WIN32)
    # Windows
    set(PLATFORM_LIBS user32 kernel32 gdi32)
endif()

# Create executable
add_executable(mmry
    src/main.cpp
)

# Link platform-specific libraries
target_link_libraries(mmry ${PLATFORM_LIBS})

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(mmry PRIVATE -Wall -Wextra -O2)
endif()

# Set output directory
set_target_properties(mmry PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)