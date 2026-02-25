#!/bin/bash
set -e

# ZonaiAnvil - Build Script
# This script verifies dependencies and builds the project.
# Usage: ./scripts/build.sh [-c] [-v]
#   -c : Clear build files (clean) before building.
#   -v : Verbose mode (show all command output).

echo "--- ZonaiAnvil Build Process ---"

# Handle flags
CLEAN_MODE=false
VERBOSE_MODE=false
while getopts "cv" opt; do
  case $opt in
    c) CLEAN_MODE=true ;;
    v) VERBOSE_MODE=true ;;
    *) echo "Usage: $0 [-c] [-v]"; exit 1 ;;
  esac
done

# Helper for suppression
REDIRECT="/dev/null"
if [ "$VERBOSE_MODE" = true ]; then
    REDIRECT="/dev/stdout"
fi

# 0. Clean build files if requested
if [ "$CLEAN_MODE" = true ]; then
    echo "[0/4] Cleaning build artifacts..."
    rm -rf build CMakeCache.txt CMakeFiles > $REDIRECT 2>&1
    echo "  - Clean complete."
fi

# 1. Check for required build tools
echo "[1/4] Checking build tools..."
REQUIRED_TOOLS=("cmake" "make" "g++" "pkg-config")
for tool in "${REQUIRED_TOOLS[@]}"; do
    if ! command -v "$tool" &> /dev/null; then
        echo "[!] Error: '$tool' is not installed."
        exit 1
    fi
    echo "  - $tool found."
done

# 2. Check for required header-only libraries in third_party
echo "[2/4] Verifying third-party headers..."
if [ ! -f "third_party/sml.hpp" ] || [ ! -f "third_party/raygui.h" ]; then
    echo "[!] Error: Missing required headers in third_party/."
    exit 1
fi
echo "  - sml.hpp found."
echo "  - raygui.h found."

# 3. Configure the project with CMake
echo "[3/4] Configuring project with CMake..."
mkdir -p build
cd build
if [ "$VERBOSE_MODE" = true ]; then
    cmake ..
else
    cmake .. > /dev/null 2>&1
fi

# 4. Build the project
echo "[4/4] Compiling..."
if [ "$VERBOSE_MODE" = true ]; then
    make -j$(nproc)
else
    make -j$(nproc) > /dev/null 2>&1
fi

echo ""
echo "--- Build Successful! ---"
echo "Executable location: build/ZonaiAnvil"
echo "Run it using: ./build/ZonaiAnvil"
