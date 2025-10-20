# A Simple C++ Allocator



## 🚀 Quick Start

```bash
# Build the project
./build.sh

# Run tests
./build.sh test

# Build with sanitizer (recommended for development)
./build.sh sanitize address
cd out && ctest --output-on-failure
```

## 🛠️ Code Quality Tools

This project uses industry-standard tools for code quality:

- **clang-format**: Auto-format code (Google C++ Style)
- **clang-tidy**: Static analysis and linting
- **Sanitizers**: Runtime bug detection (ASan, UBSan, TSan, MSan, LSan)

### Quick Commands:

```bash
./build.sh format        # Format all code
./build.sh format-check  # Check formatting
./build.sh lint          # Run linter
./build.sh sanitize address  # Build with AddressSanitizer
```

📚 **See [QUICKREF.md](QUICKREF.md) for quick reference**  
📖 **See [TOOLING.md](TOOLING.md) for detailed documentation**

## 📋 Build Script Commands

```bash
./build.sh help          # Show all available commands
./build.sh clean         # Remove build directory
./build.sh build         # Build project
./build.sh test [FILTER] # Run tests (optional: filter by name)
./build.sh run           # Run main executable
./build.sh format        # Format all source files
./build.sh lint          # Run clang-tidy linter
./build.sh sanitize [TYPE] # Build with sanitizer (TYPE: address, thread, undefined, memory, leak)
```
