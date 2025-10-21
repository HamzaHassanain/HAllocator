# HAllocator Developer Documentation

## Table of Contents

- [Overview](#overview)
- [Architecture](#architecture)
  - [Core Components](#core-components)
    - [1. Block](#1-block-hallocincludesblockhpp)
    - [2. BlocksContainer](#2-blockscontainer-hallocincludesblockscontainerhpp)
    - [3. Halloc](#3-halloc-hallocincludeshallochpp)
    - [4. RBTreeDriver](#4-rbtreedriver-rb-tree)
- [Implementation Details](#implementation-details)
  - [Memory Layout](#memory-layout)
  - [Allocation Algorithm](#allocation-algorithm)
  - [Deallocation and Coalescing](#deallocation-and-coalescing)
  - [Red-Black Tree Properties](#red-black-tree-properties)
- [Usage Examples](#usage-examples)
  - [Basic Block Usage](#basic-block-usage)
  - [Using BlocksContainer](#using-blockscontainer)
  - [STL Container Integration](#stl-container-integration)
  - [Custom Allocator Example](#custom-allocator-example)
- [Testing](#testing)
  - [Running Tests](#running-tests)
  - [Test Coverage](#test-coverage)
  - [Writing New Tests](#writing-new-tests)
- [Performance Characteristics](#performance-characteristics)
  - [Time Complexity](#time-complexity)
  - [Space Complexity](#space-complexity)
  - [Optimization Tips](#optimization-tips)
- [Building from Source](#building-from-source)
  - [Prerequisites](#prerequisites)
  - [Build Steps](#build-steps)
  - [CMake Options](#cmake-options)
- [Development Tools](#development-tools)
  - [scripts.sh - Build & Development Helper](#scriptssh---build--development-helper)
  - [clang-format - Code Formatting](#clang-format---code-formatting)
  - [clang-tidy - Static Analysis](#clang-tidy---static-analysis)
  - [clangd - Language Server](#clangd---language-server)
- [Debugging and Troubleshooting](#debugging-and-troubleshooting)
  - [Common Issues](#common-issues)
  - [Debug Macros](#debug-macros)
- [API Reference](#api-reference)
  - [Key Classes](#key-classes)
  - [Key Functions](#key-functions)
- [Contributing](#contributing)
  - [Development Workflow](#development-workflow)
  - [Code Style](#code-style)
- [License](#license)
- [Contact](#contact)
- [Acknowledgments](#acknowledgments)

---

## Overview

HAllocator is a modern C++ memory allocator implementation that provides efficient memory management through advanced data structures and algorithms. This documentation is intended for developers who want to understand the internals, extend the functionality, or integrate HAllocator into their projects.

### Key Features

- **Best-Fit Allocation**: Uses a Red-Black Tree for O(log n) best-fit search
- **Memory Coalescing**: Automatically merges adjacent free blocks using a doubly-linked list
- **Multi-Block Management**: High-level container for managing multiple memory blocks
- **STL Compatibility**: Fully compatible with STL containers as a custom allocator
- **Thread-Safe Operations**: (if enabled) Safe for concurrent usage
- **Zero External Dependencies**: Pure C++23 implementation

---

## Architecture

### Core Components

#### 1. **Block** (`halloc/includes/Block.hpp`)

The fundamental unit of memory management. Each Block represents a contiguous region of memory that can be subdivided into smaller allocations.

**Key Responsibilities:**

- Manages a single large memory region
- Tracks free/used regions using a Red-Black Tree
- Handles allocation and deallocation requests
- Performs memory coalescing

**Public API:**

```cpp
class Block {
public:
    explicit Block(size_t size);
    void* allocate(size_t size, void* hint = nullptr);
    void deallocate(void* ptr, size_t size);
    void* best_fit(size_t size);
    size_t available() const;
    void print_stats() const;
};
```

#### 2. **BlocksContainer** (`halloc/includes/BlocksContainer.hpp`)

A container that manages multiple Block instances, providing a higher-level allocation interface.

**Key Responsibilities:**

- Manages multiple memory blocks
- Routes allocation requests to appropriate blocks
- Expands capacity by adding new blocks when needed
- Provides aggregated statistics

**Template Parameters:**

```cpp
template<size_t BlockSize, size_t MaxBlocks>
class BlocksContainer {
    // ...
};
```

#### 3. **Halloc** (`halloc/includes/Halloc.hpp`)

The main allocator interface that provides STL-compatible memory allocation.

**Key Responsibilities:**

- STL allocator interface implementation
- Type-safe allocation/deallocation
- Compatible with std::vector, std::list, std::set, etc.
- Manages object construction and destruction

**Template Parameters:**

```cpp
template<typename T, size_t BlockSize = 1024 * 1024, size_t MaxBlocks = 4>
class Halloc {
    // STL allocator interface
};
```

#### 4. **RBTreeDriver** (`rb-tree/`)

A Red-Black Tree implementation optimized for memory region management.

**Key Responsibilities:**

- Maintains balanced tree of free memory regions
- Provides O(log n) search, insert, and delete operations
- Supports custom comparison for best-fit search
- Memory-efficient node storage

---

## Implementation Details

### Memory Layout

Each Block maintains its memory in the following structure:

```
┌─────────────────────────────────────────────────────┐
│                   Block Header                       │
│  (metadata: size, free list head, RB tree root)     │
├─────────────────────────────────────────────────────┤
│                                                      │
│              Allocatable Memory Region               │
│                                                      │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐         │
│  │  Region  │  │  Region  │  │  Region  │  ...    │
│  │  Header  │  │  Header  │  │  Header  │         │
│  ├──────────┤  ├──────────┤  ├──────────┤         │
│  │   Data   │  │   Data   │  │   Data   │         │
│  └──────────┘  └──────────┘  └──────────┘         │
│                                                      │
└─────────────────────────────────────────────────────┘
```

### Allocation Algorithm

1. **Search Phase**: Use RB tree to find best-fit free region
2. **Split Phase**: If region is larger than needed, split it
3. **Update Phase**: Update free list and RB tree
4. **Return Phase**: Return pointer to allocated memory

```cpp
// This is Just a conceptual snippet, not the actual code
void* Block::allocate(size_t size, void* hint) {
    // 1. Find best-fit region (O(log n))
    void* region = best_fit(size);

    // 2. Split if necessary
    if (region_size(region) > size + MIN_SPLIT_SIZE) {
        split_region(region, size);
    }

    // 3. Mark as used and update structures
    mark_used(region);
    remove_from_free_list(region);

    // 4. Return pointer
    return region;
}
```

### Deallocation and Coalescing

When memory is freed, HAllocator attempts to merge it with adjacent free blocks:

```cpp
// This is Just a conceptual snippet, not the actual code
void Block::deallocate(void* ptr, size_t size) {
    // 1. Mark region as free
    mark_free(ptr);

    // 2. Try to coalesce with previous block
    if (prev_is_free(ptr)) {
        ptr = coalesce_with_prev(ptr);
    }

    // 3. Try to coalesce with next block
    if (next_is_free(ptr)) {
        coalesce_with_next(ptr);
    }

    // 4. Add to free list and RB tree
    add_to_free_list(ptr);
    rb_tree.insert(ptr);
}
```

### Red-Black Tree Properties

The RB tree maintains these invariants:

1. Every node is either red or black
2. The root is black
3. All leaves (NIL) are black
4. Red nodes have black children
5. All paths from root to leaves contain the same number of black nodes

This ensures O(log n) performance for all operations.

---

## Usage Examples

### Basic Block Usage

```cpp
#include <halloc/includes/Block.hpp>

// Create a 1MB block
hh::halloc::Block block(1024 * 1024);

// Allocate memory
void* ptr1 = block.allocate(128, block.best_fit(128));
void* ptr2 = block.allocate(256, block.best_fit(256));

// Use the memory...

// Free memory
block.deallocate(ptr1, 128);
block.deallocate(ptr2, 256);
```

### Using BlocksContainer

```cpp
#include <halloc/includes/BlocksContainer.hpp>

// Container with 4 blocks of 1MB each
hh::halloc::BlocksContainer<1024 * 1024, 4> container;

// Allocate from any available block
void* ptr = container.allocate(512);

// Deallocate
container.deallocate(ptr, 512);

// Get statistics, needs to be implemented
// std::cout << "Available: " << container.total_available() << " bytes\n";
```

### STL Container Integration

```cpp
#include <halloc/includes/Halloc.hpp>
#include <vector>
#include <map>

// Vector with custom allocator
std::vector<int, hh::halloc::Halloc<int, 1024 * 1024>> vec;
vec.push_back(42);
vec.push_back(100);

// Map with custom allocator
using MyMap = std::map<
    std::string,
    int,
    std::less<std::string>,
    hh::halloc::Halloc<std::pair<const std::string, int>>
>;

MyMap myMap;
myMap["hello"] = 1;
myMap["world"] = 2;
```

### Custom Allocator Example

```cpp
#include <halloc/includes/Halloc.hpp>

// Define custom allocator for specific type
using IntAllocator = hh::halloc::Halloc<int, 512 * 1024, 2>;

// Use with STL containers
std::vector<int, IntAllocator> numbers;
for (int i = 0; i < 1000; ++i) {
    numbers.push_back(i * i);
}
```

---

## Testing

HAllocator includes comprehensive unit tests using GoogleTest.

### Running Tests

```bash
# Build with tests
cmake -S . -B build
cmake --build build

# Run all tests
cd build
ctest --output-on-failure

# Or use the helper script
./scripts.sh test
```

### Test Coverage

Tests cover:

- Basic allocation/deallocation
- Boundary conditions (empty, full blocks)
- Coalescing behavior
- RB tree operations
- STL container integration
- Multi-threaded scenarios (if enabled)
- Memory corruption detection

### Writing New Tests

Example test structure:

```cpp
#include <gtest/gtest.h>
#include <halloc/includes/Block.hpp>

TEST(BlockTest, BasicAllocation) {
    hh::halloc::Block block(1024);

    void* ptr = block.allocate(128, nullptr);
    ASSERT_NE(ptr, nullptr);

    block.deallocate(ptr, 128);
    EXPECT_EQ(block.available(), 1024);
}
```

---

## Performance Characteristics

### Time Complexity

| Operation  | Best Case | Average Case | Worst Case |
| ---------- | --------- | ------------ | ---------- |
| Allocate   | O(1)      | O(log n)     | O(log n)   |
| Deallocate | O(1)      | O(log n)     | O(log n)   |
| Best-Fit   | O(log n)  | O(log n)     | O(log n)   |
| Coalesce   | O(1)      | O(1)         | O(1)       |

### Space Complexity

- **Per Block**: O(1) overhead + O(n) for free regions tracking
- **RB Tree**: O(n) where n is number of free regions
- **Free List**: O(n) for doubly-linked list of free regions

### Optimization Tips

1. **Pre-allocate Blocks**: Create BlocksContainer with sufficient capacity
2. **Size Classes**: Use multiple allocators for different size ranges
3. **Alignment**: Request aligned memory for better cache performance
4. **Batch Operations**: Allocate multiple objects at once when possible

---

## Building from Source

### Prerequisites

- CMake 3.10+
- C++23 compatible compiler (GCC 11+, Clang 14+, MSVC 19.30+)
- Git

### Build Steps

```bash
# Clone the repository
git clone https://github.com/HamzaHassanain/HAllocator.git
cd HAllocator

# Configure with CMake
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --parallel

# Install (optional)
sudo cmake --install build
```

### CMake Options

```cmake
# Enable/disable tests
-DBUILD_TESTING=ON

# Enable documentation generation
-DBUILD_DOC=ON

# Set build type
-DCMAKE_BUILD_TYPE=Release|Debug|RelWithDebInfo

# Enable sanitizers (debug builds)
-DENABLE_ASAN=ON    # Address Sanitizer
-DENABLE_UBSAN=ON   # Undefined Behavior Sanitizer
-DENABLE_TSAN=ON    # Thread Sanitizer
```

---

## Development Tools

HAllocator uses a comprehensive set of modern development tools to ensure code quality, consistency, and maintainability. This section covers all the essential tools integrated into the project.

### scripts.sh - Build & Development Helper

The `scripts.sh` is a unified command-line interface for all common development tasks. It simplifies building, testing, formatting, linting, and debugging operations.

#### Available Commands

##### 1. **clean** - Remove Build Artifacts

```bash
./scripts.sh clean
```

- Removes the entire `build/` directory
- Useful for fresh builds or cleaning up disk space
- Often combined with build: `./scripts.sh clean build`

##### 2. **build** - Build the Project

```bash
./scripts.sh build
# or simply
./scripts.sh
```

- Creates and configures the `build/` directory if it doesn't exist
- Runs CMake configuration
- Builds the project using all available CPU cores (`-j$(nproc)`)
- **Default command** if no arguments are provided

**Output:**

```
Building project...
Configuring CMake...
Building project...
Build completed successfully!
```

##### 3. **test** - Run Unit Tests

```bash
# you have to build first
./scripts.sh build

# Run all tests
./scripts.sh test


# Run tests matching a pattern
./scripts.sh build test BasicAllocatorTest

./scripts.sh build test "STRESS_*"

# Run only small/fast tests
./scripts.sh build test small-only
```

- Runs all GoogleTest unit tests using CTest
- Supports filtering tests by name pattern
- Provides detailed output on failures (`--output-on-failure`)
- **Note:** Must build first before running tests

##### 4. **format** - Format Code with clang-format

```bash
./scripts.sh format
```

- Automatically formats all `.cpp`, `.hpp`, `.h`, and `.ipp` files
- Uses Google C++ Style Guide with custom modifications
- Applies formatting in-place (modifies files)
- Excludes `build/` and `out/` directories
- **Should be run before every commit**

**What it does:**

- Fixes indentation (4 spaces)
- Aligns code consistently
- Applies proper spacing around operators
- Organizes includes
- Wraps lines at 100 characters

##### 5. **format-check** - Verify Code Formatting

```bash
./scripts.sh format-check
```

- Checks if code is properly formatted **without modifying files**
- Exits with error code if formatting is needed
- Perfect for CI/CD pipelines
- Lists all files that need formatting

**Exit codes:**

- `0` - All files properly formatted
- `1` - Some files need formatting

**Output:**

```
All files are properly formatted!
```

or

```
Some files need formatting:
halloc/src/Block.cpp
rb-tree/rb-tree.hpp

Run './scripts.sh format' to fix formatting issues.
```

##### 6. **lint** - Run Static Analysis with clang-tidy

```bash
./scripts.sh lint
```

- Performs comprehensive static code analysis
- Checks for bugs, performance issues, and style violations
- Uses `compile_commands.json` for accurate analysis
- Examines all source/header files except tests and examples
- **Recommended to run before pull requests**

**What it checks:**

- Potential bugs and logic errors
- Performance inefficiencies
- Modern C++ best practices
- Code style consistency
- Memory safety issues
- Undefined behavior risks

##### 7. **sanitize** - Build with Sanitizers

```bash
# Address Sanitizer (default) - detects memory errors
./scripts.sh sanitize
./scripts.sh sanitize address

# Thread Sanitizer - detects data races
./scripts.sh sanitize thread

# Undefined Behavior Sanitizer - detects UB
./scripts.sh sanitize undefined

# Memory Sanitizer - detects uninitialized memory reads
./scripts.sh sanitize memory

# Leak Sanitizer - detects memory leaks
./scripts.sh sanitize leak
```

- Builds the project with compiler sanitizers enabled
- Essential for finding runtime bugs during development
- **Always run tests with sanitizers before releasing**

**After building with sanitizer:**

```bash
cd build
ctest --output-on-failure
```

**Sanitizer Types:**

| Sanitizer     | Flag                   | Detects                                        |
| ------------- | ---------------------- | ---------------------------------------------- |
| **Address**   | `-fsanitize=address`   | Buffer overflows, use-after-free, memory leaks |
| **Thread**    | `-fsanitize=thread`    | Data races, deadlocks                          |
| **Undefined** | `-fsanitize=undefined` | Undefined behavior, integer overflow           |
| **Memory**    | `-fsanitize=memory`    | Uninitialized memory reads                     |
| **Leak**      | `-fsanitize=leak`      | Memory leaks only                              |

##### 8. **help** - Show Usage Information

```bash
./scripts.sh help
./scripts.sh -h
./scripts.sh --help
```

- Displays comprehensive usage information
- Lists all available commands with examples

#### Common Workflows

**Starting fresh:**

```bash
./scripts.sh clean build test
```

**Before committing:**

```bash
./scripts.sh format
./scripts.sh lint
./scripts.sh build test
```

**Debugging memory issues:**

```bash
./scripts.sh clean
./scripts.sh sanitize address
cd build && ctest --output-on-failure
```

**Quick validation:**

```bash
./scripts.sh format-check
./scripts.sh build test small-only
```

---

### clang-format - Code Formatting

HAllocator uses **clang-format** with a customized Google C++ Style Guide for consistent code formatting.

#### Configuration File: `.clang-format`

```yaml
BasedOnStyle: Google
IndentWidth: 4 # 4-space indentation
ColumnLimit: 100 # Max line length
PointerAlignment: Left # int* ptr (not int *ptr)
BreakBeforeBraces: Attach # Opening brace on same line
```

#### Key Style Rules

1. **Indentation:** 4 spaces (no tabs)
2. **Line Length:** Maximum 100 characters
3. **Braces:** Attached style (`if (condition) {`)
4. **Pointers:** Left-aligned (`int* ptr`)
5. **Includes:** Sorted and grouped
6. **Comments:** 2 spaces before trailing comments
7. **Templates:** Always break template declarations

#### Example Formatting

**Before:**

```cpp
int*foo(int x,int y){
if(x>y)return x;
else return y;}
```

**After:**

```cpp
int* foo(int x, int y) {
    if (x > y)
        return x;
    else
        return y;
}
```

#### Integration with Editors

**VS Code:** Install "C/C++" extension and add to `settings.json`:

```json
{
  "editor.formatOnSave": true,
  "[cpp]": {
    "editor.defaultFormatter": "ms-vscode.cpptools"
  }
}
```

**Vim/Neovim:**

```vim
" Format on save
autocmd BufWritePre *.cpp,*.hpp,*.h :!clang-format -i %
```

**CLion/IntelliJ:** Settings → Editor → Code Style → Enable ClangFormat

---

### clang-tidy - Static Analysis

**clang-tidy** is a powerful static analysis tool that catches bugs, suggests modern C++ practices, and enforces code quality standards.

#### Configuration File: `.clang-tidy`

The project uses a comprehensive set of checks:

```yaml
Checks: >
  bugprone-*,              # Detect common bugs
  clang-analyzer-*,        # Deep static analysis
  cppcoreguidelines-*,     # C++ Core Guidelines
  google-*,                # Google style guide
  modernize-*,             # Modern C++ features
  performance-*,           # Performance optimization
  readability-*            # Code readability
```

#### Check Categories

##### 1. **bugprone-\*** - Bug Detection

Catches common programming errors:

- Use-after-move
- Suspicious string comparisons
- Incorrect sizeof() usage
- Forward declaration mismatches

##### 2. **clang-analyzer-\*** - Deep Analysis

Performs path-sensitive analysis:

- Null pointer dereferences
- Memory leaks
- Dead code
- Logic errors

##### 3. **cppcoreguidelines-\*** - Best Practices

Enforces C++ Core Guidelines:

- Special member functions rule of 5/0
- Proper use of const
- Avoid naked new/delete
- RAII compliance

##### 4. **google-\*** - Google Style

Google C++ style conformance:

- Naming conventions
- Include order
- Proper use of namespaces
- Readability improvements

##### 5. **modernize-\*** - Modern C++

Suggests modern C++ features:

- Use `nullptr` instead of `NULL`
- Use `override` keyword
- Use range-based for loops
- Use `auto` where appropriate

##### 6. **performance-\*** - Optimization

Identifies performance issues:

- Unnecessary copies
- Move semantics opportunities
- Inefficient string concatenation
- Container operations

##### 7. **readability-\*** - Code Clarity

Improves code readability:

- Naming conventions (snake_case, CamelCase)
- Function complexity
- Magic numbers
- Identifier length

#### Naming Conventions Enforced

```cpp
namespace my_namespace {}        // lower_case
class MyClass {};                 // CamelCase
struct MyStruct {};               // CamelCase
void my_function() {}             // lower_case
int my_variable = 0;              // lower_case
const int MY_CONSTANT = 42;       // UPPER_CASE
enum class Color { RED, GREEN };  // UPPER_CASE
template<typename T> class Vec{}; // CamelCase
```

#### Example Checks

**Before (flagged by clang-tidy):**

```cpp
int* allocate(int size) {
    int* p = (int*)malloc(size);  // C-style cast
    if (p == NULL)                 // Use nullptr
        return 0;                  // Return nullptr
    return p;                      // Raw pointer ownership
}
```

**After (clean):**

```cpp
std::unique_ptr<int[]> allocate(size_t size) {
    auto p = std::make_unique<int[]>(size);  // Modern C++
    if (p == nullptr)                         // nullptr
        return nullptr;                       // Consistent
    return p;                                 // RAII
}
```

#### Running clang-tidy Manually

```bash
# On a specific file
clang-tidy -p build halloc/src/Block.cpp

# On all files with auto-fix
clang-tidy -p build --fix-errors halloc/src/*.cpp

# With specific checks only
clang-tidy -p build -checks="modernize-*" halloc/src/Block.cpp
```

---

### clangd - Language Server

**clangd** is the language server that powers intelligent code completion, navigation, and real-time diagnostics in your editor.

#### Configuration File: `.clangd`

```yaml
CompileFlags:
  Add:
    - "-std=c++23" # C++23 standard
    - "-Wall" # All warnings
    - "-Wextra" # Extra warnings
    - "-pedantic" # Strict ISO C++

Diagnostics:
  ClangTidy:
    # Enables all checks from .clang-tidy

InlayHints:
  Enabled: Yes # Show hints in editor
  ParameterNames: Yes # Show parameter names
  DeducedTypes: Yes # Show auto types

Hover:
  ShowAKA: Yes # Show type aliases
```

#### Features

##### 1. **Code Completion**

Intelligent autocomplete based on actual code analysis:

```cpp
Block block(1024);
block.al|    // Shows: allocate(), available()
```

##### 2. **Go to Definition** (F12 / Ctrl+Click)

Jump to where functions/classes are defined:

```cpp
block.allocate(128);  // Jump to Block::allocate definition
```

##### 3. **Find References** (Shift+F12)

Find all usages of a symbol across the codebase:

```cpp
void* allocate(size_t size);  // Find all calls to allocate
```

##### 4. **Real-time Diagnostics**

Shows errors and warnings as you type:

```cpp
int* ptr = nullptr;
*ptr = 42;  // Warning: Dereferencing null pointer
```

##### 5. **Inlay Hints**

Shows parameter names and deduced types inline:

```cpp
block.allocate(128, nullptr);
              // ^      ^
              // size   hint (shown in editor)

auto ptr = get_pointer();
     // ^
     // int* (shown in editor)
```

##### 6. **Code Actions** (Ctrl+.)

Quick fixes and refactorings:

- Extract function
- Add missing includes
- Implement pure virtual methods
- Generate getters/setters

##### 7. **Symbol Outline**

View document structure:

- Classes and methods
- Functions
- Variables
- Namespaces

#### Editor Setup

**VS Code:**

1. Install "clangd" extension
2. Disable built-in IntelliSense (conflicts with clangd)
3. Add to `.vscode/settings.json`:

```json
{
  "clangd.arguments": [
    "--background-index",
    "--clang-tidy",
    "--header-insertion=iwyu"
  ]
}
```

**Neovim (with nvim-lspconfig):**

```lua
require('lspconfig').clangd.setup{
    cmd = {
        "clangd",
        "--background-index",
        "--clang-tidy",
        "--header-insertion=iwyu"
    }
}
```

**Emacs (with eglot):**

```elisp
(add-hook 'c++-mode-hook 'eglot-ensure)
```

#### Compilation Database

clangd requires `compile_commands.json` for accurate analysis:

```bash
# Generate during build
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Symlink to project root (for clangd to find it)
ln -s build/compile_commands.json .
```

#### Performance Tips

1. **Enable background indexing** for faster startup
2. **Limit cache size** if working on large codebases
3. **Use .clangd file** to configure per-project settings
4. **Keep compile_commands.json updated** after CMake changes

---

## Debugging and Troubleshooting

### Common Issues

#### Memory Leaks

Use AddressSanitizer to detect leaks:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=ON
cmake --build build
./build/bin/your_test
```

#### Segmentation Faults

Enable core dumps and use gdb:

```bash
ulimit -c unlimited
./build/bin/your_test
gdb ./build/bin/your_test core
```

#### Performance Issues

Use profiling tools:

```bash
# Valgrind (callgrind)
valgrind --tool=callgrind ./build/bin/your_test
kcachegrind callgrind.out.*

# perf (Linux)
perf record ./build/bin/your_test
perf report
```

### Debug Macros

```cpp
// Enable debug output
#define HALLOC_DEBUG 1

// Enable allocation tracking
#define HALLOC_TRACK_ALLOCATIONS 1

// Enable bounds checking
#define HALLOC_BOUNDS_CHECK 1
```

---

## API Reference

For detailed API documentation, see the [Doxygen-generated documentation](https://hamzahassanain.github.io/HAllocator/).

### Key Classes

- `hh::halloc::Block` - Single memory block manager
- `hh::halloc::BlocksContainer<Size, MaxBlocks>` - Multiple block container
- `hh::halloc::Halloc<T, Size, MaxBlocks>` - STL-compatible allocator
- `hh::halloc::RBTreeDriver` - Red-Black tree implementation

### Key Functions

```cpp
// Block operations
void* allocate(size_t size, void* hint = nullptr);
void deallocate(void* ptr, size_t size);
void* best_fit(size_t size);
size_t available() const;

// Container operations
void* allocate(size_t size);
void deallocate(void* ptr, size_t size);
size_t total_available() const;
size_t num_blocks() const;

// STL Allocator operations
T* allocate(size_t n);
void deallocate(T* p, size_t n);
template<typename... Args> void construct(T* p, Args&&... args);
void destroy(T* p);
```

---

## Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

### Development Workflow

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Run tests (`./scripts.sh test`)
5. Format code (`./scripts.sh format`)
6. Run linter (`./scripts.sh lint`)
7. Commit changes (`git commit -m 'Add amazing feature'`)
8. Push to branch (`git push origin feature/amazing-feature`)
9. Open a Pull Request

### Code Style

We follow the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) with minor modifications.

Use clang-format to format your code:

```bash
./scripts.sh format
```

---

## License

HAllocator is licensed under the AGPL-3.0 License. See [LICENSE](LICENSE) for details.

---

## Contact

- **Author**: Hamza Hassanain
- **GitHub**: [@HamzaHassanain](https://github.com/HamzaHassanain)
- **Project**: [HAllocator](https://github.com/HamzaHassanain/HAllocator)
- **Documentation**: [https://hamzahassanain.github.io/HAllocator/](https://hamzahassanain.github.io/HAllocator/)

---

## Acknowledgments

- GoogleTest for unit testing framework
- Doxygen for documentation generation
- The C++ community for inspiration and best practices

---

_This documentation was generated for HAllocator. For the latest updates, visit the [GitHub repository](https://github.com/HamzaHassanain/HAllocator)._
