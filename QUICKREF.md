# Quick Reference: Code Quality Tools

## 📋 **Daily Workflow**

```bash
# 1. Before committing code
./scripts.sh format              # Auto-format all files
./scripts.sh test                # Run all tests

# 2. Weekly deep check
./scripts.sh lint                # Run static analysis
./scripts.sh sanitize address    # Build with sanitizer
cd out && ctest --output-on-failure
```

---

## 🎯 **Quick Commands**

| Command                           | Description                   |
| --------------------------------- | ----------------------------- |
| `./scripts.sh format`             | Format all code files         |
| `./scripts.sh format-check`       | Check if formatting is needed |
| `./scripts.sh lint`               | Run clang-tidy linter         |
| `./scripts.sh sanitize address`   | Build with AddressSanitizer   |
| `./scripts.sh sanitize thread`    | Build with ThreadSanitizer    |
| `./scripts.sh sanitize undefined` | Build with UBSanitizer        |
| `./scripts.sh clean build`        | Clean build from scratch      |
| `./scripts.sh test [PATTERN]`     | Run tests (optional filter)   |

---

## 🐛 **Sanitizer Quick Guide**

### When to use each:

- **AddressSanitizer** 👈 **Start here**
  - Catches: memory leaks, use-after-free, buffer overflows
  - Use for: General development and testing
- **UndefinedBehaviorSanitizer**
  - Catches: integer overflow, null derefs, unaligned access
  - Use for: Arithmetic-heavy code
- **ThreadSanitizer**
  - Catches: data races, deadlocks
  - Use for: Multi-threaded code only
- **MemorySanitizer**
  - Catches: uninitialized memory reads
  - Use for: Debugging weird behavior
- **LeakSanitizer**
  - Catches: memory leaks only (lightweight)
  - Use for: Quick leak checks

---

## 📝 **Example Output**

### ✅ Good formatting:

```
$ ./scripts.sh format-check
Checking code formatting...
✅ All files are properly formatted!
```

### ❌ Needs formatting:

```
$ ./scripts.sh format-check
Checking code formatting...
❌ Some files need formatting:
./halloc/src/Block.ipp:142:5: error: code should be clang-formatted

Run './scripts.sh format' to fix formatting issues.
```

### 🔍 Linter output:

```
$ ./scripts.sh lint
Running clang-tidy linter...
/path/to/file.cpp:42:10: warning: variable 'x' is not used [clang-diagnostic-unused-variable]
    int x = 5;
        ^
```

### 🧪 Sanitizer output:

```
$ ./scripts.sh sanitize address
$ cd out && ctest
=================================================================
==12345==ERROR: AddressSanitizer: heap-use-after-free
READ of size 4 at 0x60300000eff0 thread T0
    #0 0x4a2f89 in main /path/to/file.cpp:42
```

---

## 🔧 **Configuration Files**

| File             | Purpose                         |
| ---------------- | ------------------------------- |
| `.clang-format`  | Formatting rules (Google style) |
| `.clang-tidy`    | Linting rules and checks        |
| `.clangd`        | clangd LSP configuration        |
| `CMakeLists.txt` | Sanitizer build options         |
| `TOOLING.md`     | Detailed documentation          |

---

## 💡 **Pro Tips**

1. **Run format before every commit**

   ```bash
   ./scripts.sh format && git add -u
   ```

2. **Format only changed files** (faster)

   ```bash
   git diff --name-only | grep -E '\.(cpp|hpp)$' | xargs clang-format -i
   ```

3. **Run sanitizers on CI/CD**

   ```bash
   ./scripts.sh sanitize address && cd out && ctest --output-on-failure
   ```

4. **VS Code integration**

   - Install "clangd" extension
   - Formatting happens automatically on save
   - Linting errors show inline

5. **Check before push**
   ```bash
   ./scripts.sh format-check && \
   ./scripts.sh clean build && \
   ./scripts.sh test && \
   echo "✅ Ready to push!"
   ```

---

## 🚨 **Common Issues**

**Q: Formatting is slow**  
A: Run on changed files only (see tip #2)

**Q: Clang-tidy gives too many warnings**  
A: Edit `.clang-tidy` to disable specific checks

**Q: Sanitizer causes tests to fail**  
A: This is GOOD! It found bugs. Fix them.

**Q: "clang-format: command not found"**  
A: Install with `sudo apt install clang-format`

---

**Need more details?** → See `TOOLING.md`
