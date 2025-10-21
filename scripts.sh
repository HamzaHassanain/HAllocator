#!/bin/bash

# Exit on any error
set -e

# Ensure we're in the project root directory
cd "$(dirname "$0")"

# ==================== HELP ====================
if [ "$1" = "help" ] || [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
    echo "Usage: ./scripts.sh [COMMAND] [OPTIONS]"
    echo ""
    echo "Commands:"
    echo "  clean              - Remove build directory"
    echo "  build              - Build the project (default)"
    echo "  test [PATTERN]     - Run tests (optional: filter by pattern) or 'small-only' for small tests"
    echo "  run                - Run the main executable"
    echo "  format             - Format all source files with clang-format"
    echo "  format-check       - Check formatting without modifying files"
    echo "  lint               - Run clang-tidy linter"
    echo "  sanitize [TYPE]    - Build with sanitizer (address|thread|undefined|memory|leak)"
    echo "  help               - Show this help message"
    echo ""
    echo "Examples:"
    echo "  ./scripts.sh clean build"
    echo "  ./scripts.sh test BlockTest"
    echo "  ./scripts.sh sanitize address"
    echo "  ./scripts.sh format"
    exit 0
fi


# ==================== CLEAN ====================
if [ "$1" = "clean" ]; then
    echo "Cleaning previous build..."
    rm -rf build
    shift
fi


# ==================== NORMAL BUILD ====================
# Create build directory if it doesn't exist

if [ "$1" = "build" ] || [ -z "$1" ]; then
    echo "Building project..."
    
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
fi


# ==================== FORMAT ====================
if [ "$1" = "format" ]; then
    echo "Formatting code with clang-format (Google style)..."
    find . -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" -o -name "*.ipp" \) \
    ! -path "./out/*" ! -path "./build/*" \
    -exec clang-format -i -style=file {} +
    echo "Formatting completed!"
    exit 0
fi

if [ "$1" = "format-check" ]; then
    echo "Checking code formatting..."
    NEEDS_FORMAT=$(find . -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" -o -name "*.ipp" \) \
        ! -path "./out/*" ! -path "./build/*" \
    -exec clang-format -style=file --dry-run --Werror {} + 2>&1 || true)
    
    if [ -n "$NEEDS_FORMAT" ]; then
        echo "❌ Some files need formatting:"
        echo "$NEEDS_FORMAT"
        echo ""
        echo "Run './scripts.sh format' to fix formatting issues."
        exit 1
    else
        echo "✅ All files are properly formatted!"
        exit 0
    fi
fi

# ==================== LINT ====================
if [ "$1" = "lint" ]; then
    echo "Running clang-tidy linter..."
    
    # Build with compile_commands.json first
    mkdir -p build
    cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    
    # Run clang-tidy on source files, header files, implementation files
    find . -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" -o -name "*.ipp" \) \
    ! -path "./out/*" ! -path "./build/*" ! -path "./tests/*" ! -path "./basic-allocator/*" \
    -exec clang-tidy -p build {} \;
    
    echo "Linting completed!"
    exit 0
fi

# ==================== SANITIZER BUILD ====================
if [ "$1" = "sanitize" ]; then
    SANITIZER_TYPE=${2:-address}
    echo "Building with $SANITIZER_TYPE sanitizer..."
    
    mkdir -p build
    cmake -S . -B build -DSANITIZER=$SANITIZER_TYPE
    cd build
    make -j$(nproc)
    cd ..
    
    echo "Build with sanitizer completed!"
    echo "Run tests with: cd build && ctest --output-on-failure"
    exit 0
fi

# ==================== TEST ====================
if [ "$1" = "test" ]; then
    echo "Running tests..."
    cd build
    if [ -n "$2" ]; then
        if [ "$2" = "small-only" ]; then
            ctest -R "SMALL_" --output-on-failure
            exit 0
        else
            ctest -R "$2" --output-on-failure
        fi
    else
        ctest --output-on-failure
    fi
    cd ..
fi
