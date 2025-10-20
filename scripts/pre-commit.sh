#!/bin/bash
#
# Git pre-commit hook to enforce code quality
# Install: cp scripts/pre-commit.sh .git/hooks/pre-commit && chmod +x .git/hooks/pre-commit
#

echo "🔍 Running pre-commit checks..."

# Check formatting
echo "📝 Checking code formatting..."
if ! ./build.sh format-check; then
    echo ""
    echo "❌ Code formatting check failed!"
    echo "💡 Run './build.sh format' to fix formatting issues."
    echo "   Or commit with --no-verify to skip this check."
    exit 1
fi

# Optional: Run quick tests
echo "🧪 Running quick tests..."
if ! ./build.sh test 2>&1 | grep -q "100% tests passed"; then
    echo ""
    echo "❌ Tests failed!"
    echo "💡 Fix the failing tests or commit with --no-verify to skip."
    exit 1
fi

echo "✅ All pre-commit checks passed!"
exit 0
