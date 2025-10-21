# Documentation Setup Guide

This document explains the Doxygen setup for the HAllocator project.

## What Was Set Up

1. **Doxyfile** - Doxygen configuration file with the following settings:

   - Project name: HAllocator
   - Project brief: A C++ memory allocator using Red-Black trees
   - Output directory: `docs/`
   - Input directories: `README.md`, `halloc/`, `rb-tree/`, `basic-allocator/`
   - Recursive scanning enabled
   - README.md set as main page
   - Excludes build/, tests/, and .git/ directories

2. **docs/.nojekyll** - Empty file that tells GitHub Pages not to process the site with Jekyll

3. **CMakeLists.txt** - Added optional Doxygen target:

   ```bash
   cmake -S . -B build -DBUILD_DOC=ON
   cmake --build build --target doc
   ```

4. **.github/workflows/docs.yml** - GitHub Actions workflow that:

   - Builds documentation on every push to master/main
   - Deploys to GitHub Pages automatically
   - Also runs on pull requests (build only, no deploy)

5. **README.md** - Updated with:
   - Link to online documentation
   - Instructions for building docs locally

## Local Usage

### Generate Documentation Locally

```bash
# Using Doxygen directly
doxygen Doxyfile

# Or using CMake
cmake -S . -B build -DBUILD_DOC=ON
cmake --build build --target doc
```

The documentation will be generated in `docs/html/`. Open `docs/html/index.html` in a browser.

### Prerequisites

```bash
# On Debian/Ubuntu
sudo apt-get install doxygen graphviz

# On macOS
brew install doxygen graphviz
```

## GitHub Pages Setup

To enable GitHub Pages for your repository:

1. Go to your repository on GitHub
2. Click **Settings** → **Pages**
3. Under **Source**, select **GitHub Actions**
4. Push the new files to your repository

Once the workflow runs, your documentation will be available at:
https://hamzahassanain.github.io/memory-allocator-and-smart-ptr/

## Workflow Triggers

The documentation workflow runs:

- On push to `master` or `main` branch → builds and deploys
- On pull requests → builds only (no deployment)
- Manually via workflow_dispatch

## Files Added

- `Doxyfile` - Doxygen configuration
- `docs/.nojekyll` - GitHub Pages configuration
- `.github/workflows/docs.yml` - GitHub Actions workflow
- `DOCS_SETUP.md` - This file

## Files Modified

- `CMakeLists.txt` - Added Doxygen build target
- `README.md` - Added documentation section

## Maintenance

### Updating Documentation

The documentation is automatically rebuilt and deployed when you push to master/main. No manual steps required!

### Customizing Doxygen

Edit the `Doxyfile` to customize:

- Project name/version
- Input/output directories
- Theme and styling
- Graph generation
- Output formats (HTML, LaTeX, etc.)

### Troubleshooting

**Q: Documentation not updating on GitHub Pages?**

- Check the Actions tab for workflow errors
- Ensure GitHub Pages is configured to use GitHub Actions
- Verify the workflow has write permissions

**Q: Graphs not showing?**

- Install graphviz: `sudo apt-get install graphviz`
- Check HAVE_DOT setting in Doxyfile

**Q: Missing files in documentation?**

- Check INPUT paths in Doxyfile
- Verify RECURSIVE = YES
- Check EXCLUDE_PATTERNS
