#!/bin/bash

# Exit on any error
set -e

echo "Building Hamza-Allocator..."

# Ensure we're in the project root directory
cd "$(dirname "$0")"

# Clean previous build if requested
if [ "$1" = "clean" ]; then
    echo "Cleaning previous build..."
    rm -rf build
fi

# Create build directory if it doesn't exist
mkdir -p build

# Configure the project with CMake
echo "Configuring CMake..."
cmake -S . -B build

# Build the project
echo "Building project..."
cd build
make -j$(nproc)
cd ..

echo "Build completed successfully!"
echo "Executable location: build/hh_alloc"

# Optionally run the executable if 'run' argument is passed
if [ "$1" = "run" ]; then
    echo "Running the application..."
    ./build/hh_alloc
fi


if [ "$1" = "test" ]; then
    echo "Running tests..."
    cd build
    ctest --output-on-failure
    cd ..
fi