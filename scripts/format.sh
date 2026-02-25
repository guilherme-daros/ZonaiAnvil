#!/bin/bash
# ZonaiAnvil - Code Formatter Script

echo "--- ZonaiAnvil Code Formatting ---"

# Find all C++ files and format them using clang-format
# Excludes third_party to avoid modifying external libraries
find apps src include -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" -o -name "*.c" \) | xargs clang-format -i --style=file

echo "Formatting complete!"
