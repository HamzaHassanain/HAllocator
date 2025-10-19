#!/bin/bash

# Exit on any error
set -e

echo "Building Allocators..."

# Ensure we're in the project root directory
cd "$(dirname "$0")"

# Clean previous build if requested
if [ "$1" = "clean" ]; then
    echo "Cleaning previous build..."
    rm -rf out
fi




# Create build directory if it doesn't exist
mkdir -p out

# Configure the project with CMake
echo "Configuring CMake..."
cmake -S . -B out

# Build the project
echo "Building project..."
cd out
make -j$(nproc)
cd ..

echo "Build completed successfully!"

# pass the rest of the arguments to ctest
if [ "$1" = "test" ]; then
    echo "Running tests..."
    cd out
    if [ -n "$2" ]; then
        ctest -R "$2" --output-on-failure
    else
        ctest --output-on-failure
    fi
    cd ..
fi


if [ "$1" = "run" ]; then
    echo "Running hh_alloc..."
    ./out/hh_alloc
fi