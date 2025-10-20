# A Simple C++ Allocator

## 🚀 Quick Start

```bash
# Build the project
./scripts.sh

# Run tests
./scripts.sh test

# Build with sanitizer (recommended for development)
./scripts.sh sanitize address
cd out && ctest --output-on-failure
```

## 🛠️ Code Quality Tools

This project uses industry-standard tools for code quality:

- **clang-format**: Auto-format code (Google C++ Style)
- **clang-tidy**: Static analysis and linting
- **Sanitizers**: Runtime bug detection (ASan, UBSan, TSan, MSan, LSan)

### Quick Commands:

```bash
./scripts.sh format        # Format all code
./scripts.sh format-check  # Check formatting
./scripts.sh lint          # Run linter
./scripts.sh sanitize address  # Build with AddressSanitizer
```

📚 **See [QUICKREF.md](QUICKREF.md) for quick reference**  
📖 **See [TOOLING.md](TOOLING.md) for detailed documentation**

## 📋 Build Script Commands

```bash
./scripts.sh help          # Show all available commands
./scripts.sh clean         # Remove build directory
./scripts.sh build         # Build project
./scripts.sh test [FILTER] # Run tests (optional: filter by name)
./scripts.sh run           # Run main executable
./scripts.sh format        # Format all source files
./scripts.sh lint          # Run clang-tidy linter
./scripts.sh sanitize [TYPE] # Build with sanitizer (TYPE: address, thread, undefined, memory, leak)
```
