#!/bin/bash

# Script to apply clang-format to library headers and example sketches
# Uses the .clang-format configuration in the project root

set -e

# Get the project root directory (parent of scripts directory)
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "Formatting ADS1X15 library files..."
echo "Project root: $PROJECT_ROOT"
echo

# Check if clang-format is available
if ! command -v clang-format &> /dev/null; then
    echo "Error: clang-format is not installed or not in PATH"
    exit 1
fi

# Format library header files
echo "Formatting library headers (.h files)..."
find "$PROJECT_ROOT/src" -type f -name "*.h" -print0 | while IFS= read -r -d '' file; do
    echo "  Formatting: $file"
    clang-format -i "$file"
done

echo

# Format example sketches
echo "Formatting example sketches (.ino files)..."
find "$PROJECT_ROOT/examples" -type f -name "*.ino" -print0 | while IFS= read -r -d '' file; do
    echo "  Formatting: $file"
    clang-format -i "$file"
done

echo
echo "Done! All files formatted."