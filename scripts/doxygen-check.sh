#!/bin/bash

# Script to validate documentation using Doxygen
# Generates warnings only, no documentation output

set -e

# Get the project root directory (parent of scripts directory)
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "Checking ADS1X15 documentation with Doxygen..."
echo "Project root: $PROJECT_ROOT"
echo

# Check if doxygen is available
if ! command -v doxygen &> /dev/null; then
    echo "Error: doxygen is not installed or not in PATH"
    echo "Install with: sudo apt-get install doxygen (Ubuntu/Debian)"
    echo "           or: brew install doxygen (macOS)"
    exit 1
fi

# Change to project root
cd "$PROJECT_ROOT"

# Remove old warning file if it exists
rm -f doxy.warn

# Run Doxygen
echo "Running Doxygen (warnings only)..."
doxygen Doxyfile

# Check if warning file was created and has content
if [ -f doxy.warn ]; then
    WARN_COUNT=$(wc -l < doxy.warn)
    echo
    echo "======================================"
    echo "Documentation check complete!"
    echo "Warnings: $WARN_COUNT"
    echo "======================================"
    echo

    if [ $WARN_COUNT -gt 0 ]; then
        echo "Warning details (doxy.warn):"
        cat doxy.warn
        echo
        echo "Fix these warnings to improve documentation quality."
        exit 1
    else
        echo "No documentation warnings found!"
        exit 0
    fi
else
    echo "No warnings generated."
    exit 0
fi
