# Contributing to HAllocator

Thank you for your interest in contributing to HAllocator! We welcome contributions from the community and are grateful for your support.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [How to Contribute](#how-to-contribute)
- [Coding Standards](#coding-standards)
- [Testing Guidelines](#testing-guidelines)
- [Pull Request Process](#pull-request-process)
- [Reporting Bugs](#reporting-bugs)
- [Suggesting Enhancements](#suggesting-enhancements)
- [Questions](#questions)

## Code of Conduct

This project and everyone participating in it is expected to uphold a respectful and inclusive environment. Please be kind, respectful, and constructive in all interactions.

## Getting Started

1. **Fork the repository** on GitHub
2. **Clone your fork** locally:
   ```bash
   git clone https://github.com/YOUR_USERNAME/HAllocator.git
   cd HAllocator
   ```
3. **Add upstream remote**:
   ```bash
   git remote add upstream https://github.com/HamzaHassanain/HAllocator.git
   ```
4. **Create a branch** for your contribution:
   ```bash
   git checkout -b feature/your-feature-name
   ```

## Development Setup

### Prerequisites

- **C++23** compatible compiler (GCC 11+, Clang 14+, or MSVC 19.30+)
- **CMake** 3.10 or higher
- **Git**
- **clang-format** (for code formatting)
- **clang-tidy** (for static analysis)

### Building the Project

```bash
# Build the project
./scripts.sh build

# Run tests
./scripts.sh test

# Format code
./scripts.sh format

# Run static analysis
./scripts.sh lint
```

### Development Tools

We use several tools to maintain code quality:

- **clang-format**: Automatic code formatting (Google C++ Style with modifications)
- **clang-tidy**: Static analysis and linting
- **clangd**: Language server for IDE integration
- **GoogleTest**: Unit testing framework

See [DOCUMENTATION.md](DOCUMENTATION.md#dev-tools) for detailed information about these tools.

## How to Contribute

### Types of Contributions

We welcome many types of contributions:

- **Bug fixes**: Fix issues reported in the issue tracker
- **New features**: Add new functionality to the allocator
- **Performance improvements**: Optimize existing code
- **Documentation**: Improve or add documentation
- **Tests**: Add test coverage for existing features
- **Examples**: Provide usage examples
- **Build improvements**: Enhance the build system

### Contribution Workflow

1. **Check existing issues** to see if someone is already working on it
2. **Create an issue** if one doesn't exist (for bugs or feature requests)
3. **Discuss** your approach in the issue before starting major work
4. **Write your code** following our coding standards
5. **Add tests** for new functionality
6. **Update documentation** if needed
7. **Submit a pull request**

## Coding Standards

### Style Guide

We follow the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) with these modifications:

- **Indentation**: 4 spaces (not 2)
- **Line length**: Maximum 100 characters
- **Naming conventions**:
  - `snake_case` for functions and variables
  - `CamelCase` for classes and structs
  - `UPPER_CASE` for constants and enum values
  - `lower_case` for namespaces

### Code Formatting

**Always format your code before committing:**

```bash
./scripts.sh format
```

Or check formatting without modifying files:

```bash
./scripts.sh format-check
```

### Example Code Style

```cpp
namespace hh::halloc {

class MyAllocator {
  public:
    // Constructor
    explicit MyAllocator(size_t size);

    // Allocate memory
    void* allocate(size_t bytes);

    // Deallocate memory
    void deallocate(void* ptr, size_t bytes);

  private:
    size_t total_size_;
    void* memory_pool_;

    // Helper function
    bool is_valid_pointer(void* ptr) const;
};

}  // namespace hh::halloc
```

### Static Analysis

Run clang-tidy to catch potential issues:

```bash
./scripts.sh lint
```

Fix any warnings before submitting your PR.

### Modern C++ Practices

- Use **RAII** for resource management
- Prefer **smart pointers** over raw pointers for ownership
- Use **`nullptr`** instead of `NULL` or `0`
- Use **`override`** for virtual function overrides
- Use **`const`** wherever appropriate
- Use **range-based for loops** when possible
- Use **`auto`** for complex or obvious types

## Testing Guidelines

### Writing Tests

All new features and bug fixes must include tests. We use GoogleTest for unit testing.

**Test file naming**: `test_<component>.cpp`

**Example test**:

```cpp
#include <gtest/gtest.h>
#include <halloc/includes/Block.hpp>

namespace hh::halloc::test {

TEST(BlockTest, AllocateDeallocateCycle) {
    Block block(1024);

    void* ptr = block.allocate(128, nullptr);
    ASSERT_NE(ptr, nullptr);

    block.deallocate(ptr, 128);

    // Verify state after deallocation
    EXPECT_EQ(block.available(), 1024);
}

TEST(BlockTest, MultipleAllocations) {
    Block block(1024);

    void* ptr1 = block.allocate(256, nullptr);
    void* ptr2 = block.allocate(256, nullptr);

    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);
    EXPECT_NE(ptr1, ptr2);

    block.deallocate(ptr1, 256);
    block.deallocate(ptr2, 256);
}

}  // namespace hh::halloc::test
```

### Running Tests

```bash
# Run all tests
./scripts.sh build test

# Run specific test
./scripts.sh build test BlockTest

# Run with sanitizers
./scripts.sh sanitize address
cd build && ctest --output-on-failure
```

### Test Coverage

- Aim for **high test coverage** of new code
- Test **edge cases** and **boundary conditions**
- Test **error handling** paths
- Include **negative tests** (tests that should fail)

## Pull Request Process

### Before Submitting

1. **Sync with upstream**:

   ```bash
   git fetch upstream
   git rebase upstream/master
   ```

2. **Run the full validation suite**:

   ```bash
   ./scripts.sh format
   ./scripts.sh lint
   ./scripts.sh build test
   ```

3. **Run with sanitizers**:

   ```bash
   ./scripts.sh clean
   ./scripts.sh sanitize address
   cd build && ctest --output-on-failure
   ```

4. **Update documentation** if needed

5. **Write clear commit messages**:

   ```
   Short (50 chars or less) summary

   More detailed explanatory text, if necessary. Wrap it to
   about 72 characters. Explain the problem this commit solves
   and why you chose this particular solution.

   Fixes #123
   ```

### Submitting the PR

1. **Push your branch** to your fork:

   ```bash
   git push origin feature/your-feature-name
   ```

2. **Create a Pull Request** on GitHub

3. **Fill out the PR template** completely:

   - Description of changes
   - Motivation and context
   - Type of change (bug fix, feature, etc.)
   - Testing performed
   - Checklist completion

4. **Link related issues** using keywords like "Fixes #123" or "Closes #456"

### PR Review Process

- I will review your PR within a few days
- Address any feedback or requested changes
- Keep your PR updated with the latest master branch
- Once approved, I will merge your PR

### PR Checklist

- [ ] Code follows the project's style guidelines
- [ ] Code has been formatted with `./scripts.sh format`
- [ ] Static analysis passes with `./scripts.sh lint`
- [ ] All tests pass with `./scripts.sh test`
- [ ] New tests added for new functionality
- [ ] Documentation updated if needed
- [ ] Commit messages are clear and descriptive
- [ ] No merge conflicts with master branch

## Reporting Bugs

### Before Reporting

1. **Check existing issues** to avoid duplicates
2. **Try the latest version** to see if the bug is already fixed
3. **Gather information** about your environment

### Bug Report Template

When creating a bug report, include:

- **Description**: Clear and concise description of the bug
- **Steps to reproduce**: Detailed steps to reproduce the behavior
- **Expected behavior**: What you expected to happen
- **Actual behavior**: What actually happened
- **Environment**:
  - OS (Linux, Windows, macOS)
  - Compiler (GCC 11, Clang 14, etc.)
  - CMake version
  - HAllocator version/commit
- **Code sample**: Minimal reproducible example
- **Additional context**: Screenshots, logs, stack traces

## Suggesting Enhancements

### Enhancement Proposal Template

- **Title**: Clear and descriptive title
- **Problem**: What problem does this solve?
- **Proposed solution**: Describe your proposed implementation
- **Alternatives**: Alternative solutions you've considered
- **Additional context**: Examples, mockups, references

### Discussion

- Enhancement proposals should be discussed in an issue before implementation
- Large changes may require a design document
- Maintainers will provide feedback and guidance

## Questions

If you have questions about contributing:

- **Open an issue** with the "question" label
- **Check existing documentation** in [DOCUMENTATION.md](DOCUMENTATION.md)
- **Review closed issues** for similar questions

## License Note

By contributing to HAllocator, you agree that your contributions will be licensed under the same MIT License with Non-Commercial Clause as the project. This means:

- Your contributions can be used freely for non-commercial purposes
- Commercial use requires permission from the copyright holder
- You retain copyright to your contributions

---

## Recognition

Contributors will be recognized in:

- The project's README.md
- Release notes for their contributions
- GitHub's contributor statistics

Thank you for contributing to HAllocator! 🚀

---

**Project Maintainer**: [Hamza Hassanain](https://github.com/HamzaHassanain)

**Repository**: [HAllocator](https://github.com/HamzaHassanain/HAllocator)
