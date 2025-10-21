# A Simple C++ Allocator

## Contents

- [Overview](#overview)
- [Prerequisites](#prerequisites)
- [Setup](#setup)
- [Quick usage](#quick-usage)
- [Repository layout](#repository-layout)

## Overview

This repository implements a simple memory allocator in modern C++ with:

- A red-black tree for best-fit search of free regions
- A doubly linked list to coalesce adjacent free blocks
- A higher-level container that manages multiple blocks

Unit tests are provided using GoogleTest. Helper scripts wrap common tasks (build, test, lint, format, sanitizers).

## Prerequisites

Install the following packages:

### On Debian/Ubuntu Linux

```bash
sudo apt update
sudo apt install cmake build-essential clang clang-format clang-tidy git
```

### On macOS (with Homebrew)

```bash
brew install cmake llvm git
# Optionally add llvm tools to your PATH if not already:
export PATH="/opt/homebrew/opt/llvm/bin:$PATH"  # or /usr/local/opt/llvm/bin on Intel Macs
```

You need:

- CMake 3.10+
- A C++23-compatible compiler (clang++ or g++)
- make
- git
- clang-format (for formatting)
- clang-tidy (for linting)

## Setup

Use the helper script from the repository root.

```bash

git clone https://github.com/HamzaHassanain/HAllocator.git

cd HAllocator

# Optional: start fresh
./scripts.sh clean build

# Run tests
./scripts.sh test

```

If you prefer raw CMake instead of the script:

```bash
cmake -S . -B out
cmake --build out -- -j$(nproc)
```

## Quick usage

This project is a C++ allocator library, not a standalone application. To use it in your own CMake project:

1. Add this repository as a subdirectory in your CMake project:

```cmake
add_subdirectory(HAllocator)
target_link_libraries(your_target PRIVATE hallocator)

```

2. In your C++ code, include and use the allocator:

```cpp
#include <HAllocator/includes.hpp>

hh::halloc::Block block(1024 * 1024); // 1 MB block

for(size_t i = 0; i < 10; ++i) {
    void* ptr = block.allocate(128, block.best_fit(128));
    allocations.push_back(ptr);
}

for(void* ptr : allocations) {
    block.deallocate(ptr, 128);
}


```

Or use the higher-level container:

```cpp
#include <HAllocator/includes.hpp>

hh::halloc::BlocksContainer<1024 * 1024, 4> container; // 4 blocks of 1 MB each

for(size_t i = 0; i < 10; ++i) {
    void* ptr = container.allocate(256);
    allocations.push_back(ptr);
}

for(void* ptr : allocations) {
    container.deallocate(ptr, 256);
}
```

```cpp
#include <HAllocator/includes.hpp>

hh::halloc::Halloc<int ,1024 * 1024, 4> halloc; // Halloc with 4 blocks of 1 MB each

for(size_t i = 0; i < 10; ++i) {

    int* ptr = halloc.allocate(10); // allocate space for 10 integers
    memset(ptr, 0, 10 * sizeof(int)); // initialize allocated memory
    allocations.push_back(ptr);
}

for(void* ptr : allocations) {
    halloc.deallocate(ptr);
}
```

Or use with an STL container:

```cpp
#include <HAllocator/includes.hpp>
#include <vector>

std::vector<int, hh::halloc::Halloc<int, 1024 * 1024>> vec; // vector with custom allocator

for(int i = 0; i < 100; ++i) {
    vec.push_back(i);
}


std::set<int, std::less<int>, hh::halloc::Halloc<std::pair<const int, int>, 1024 * 1024, 4>> mySet;

for(int i = 0; i < 100; ++i) {
    mySet.insert({i, i * 10});
}
```

## Repository layout

- `basic-allocator/` — minimal standalone allocator example/library
- `rb-tree/` — red-black tree implementation used by the allocator
- `halloc/` — allocator library (Block, BlocksContainer, Halloc)
  - `includes/` — public headers
  - `src/` — implementation sources
- `tests/` — GoogleTest unit tests
- `CMakeLists.txt` — top-level build configuration
- `scripts.sh` — helper script for build/test/lint/format/sanitizers
