#!/bin/bash
set -e

# ZonaiAnvil - Dependencies Fetcher Script
# Targets: Debian/Ubuntu, Arch Linux

echo "--- ZonaiAnvil Dependency Setup ---"

# 1. Detect OS and install system libraries
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$ID
    OS_LIKE=$ID_LIKE
fi

echo "[1/3] Detecting OS: $OS (ID_LIKE: $OS_LIKE)"

if [[ "$OS" == "ubuntu" || "$OS" == "debian" || "$OS_LIKE" == *"debian"* ]]; then
    echo "Installing system libraries via apt... (this may take a moment)"
    sudo apt-get update -y > /dev/null 2>&1
    sudo apt-get install -y \
        build-essential \
        cmake \
        git \
        curl \
        pkg-config \
        libx11-dev \
        libxcursor-dev \
        libxinerama-dev \
        libxrandr-dev \
        libxi-dev \
        libgl1-mesa-dev \
        libglu1-mesa-dev > /dev/null 2>&1
elif [[ "$OS" == "arch" || "$OS_LIKE" == *"arch"* ]]; then
    echo "Installing system libraries via pacman... (this may take a moment)"
    sudo pacman -S --needed --noconfirm \
        base-devel \
        cmake \
        git \
        curl \
        pkgconf \
        libx11 \
        libxcursor \
        libxinerama \
        libxrandr \
        libxi \
        mesa \
        glu > /dev/null 2>&1
else
    echo "[!] OS not explicitly supported for auto-install. Please install system libraries manually."
    echo "Check DEPENDENCIES.md for a list of requirements."
fi

# 2. Ensure project directories exist
echo "[2/3] Setting up directory structure..."
mkdir -p third_party include src build

# 3. Download header-only libraries
echo "[3/3] Fetching header-only libraries into third_party/..."

# boost-ext/sml
if [ ! -f "third_party/sml.hpp" ]; then
    echo "  - Downloading boost-ext/sml.hpp"
    curl -sL https://raw.githubusercontent.com/boost-ext/sml/master/include/boost/sml.hpp -o third_party/sml.hpp
else
    echo "  - sml.hpp already exists, skipping."
fi

# raygui
if [ ! -f "third_party/raygui.h" ]; then
    echo "  - Downloading raygui.h"
    curl -sL https://raw.githubusercontent.com/raysan5/raygui/master/src/raygui.h -o third_party/raygui.h
else
    echo "  - raygui.h already exists, skipping."
fi

# Styles
echo "  - Fetching official styles..."
mkdir -p third_party/styles
STYLES=("cyber" "dark" "terminal" "bluish" "lavanda" "sunny" "cherry" "ashes")
for style in "${STYLES[@]}"; do
    if [ ! -f "third_party/styles/style_$style.h" ]; then
        echo "    * Downloading style_$style.h"
        curl -sL "https://raw.githubusercontent.com/raysan5/raygui/master/styles/$style/style_$style.h" -o "third_party/styles/style_$style.h"
    fi
done

echo "--- Setup Complete! ---"
echo "You can now build the project using: ./scripts/build.sh"
