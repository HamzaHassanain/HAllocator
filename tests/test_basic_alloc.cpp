#include <gtest/gtest.h>

#include <algorithm>
#include <cstring>
#include <random>
#include <vector>

#include "../basic-allocator/basic_alloc.hpp"

class AllocatorTest : public ::testing::Test {
protected:
    void SetUp() override {}

    void TearDown() override {}
};

// Test basic allocation
TEST_F(AllocatorTest, BasicAllocation) {
    void* ptr = hh::basic_alloc::try_alloc(100);
    ASSERT_NE(ptr, nullptr);

    // Write to the allocated memory
    memset(ptr, 0xAB, 100);

    // Verify we can read it back
    unsigned char* data = (unsigned char*)ptr;
    for (int i = 0; i < 100; i++) {
        EXPECT_EQ(data[i], 0xAB);
    }

    hh::basic_alloc::free(ptr);
}

// Test allocation of zero size
TEST_F(AllocatorTest, ZeroSizeAllocation) {
    void* ptr = hh::basic_alloc::try_alloc(0);
    EXPECT_EQ(ptr, nullptr);
}

// Test multiple allocations
TEST_F(AllocatorTest, MultipleAllocations) {
    std::vector<void*> ptrs;

    for (int i = 0; i < 10; i++) {
        void* ptr = hh::basic_alloc::try_alloc(64);
        ASSERT_NE(ptr, nullptr);
        ptrs.push_back(ptr);

        // Write unique pattern to each allocation
        memset(ptr, i, 64);
    }

    // Verify all allocations are unique and contain correct data
    for (size_t i = 0; i < ptrs.size(); i++) {
        unsigned char* data = (unsigned char*)ptrs[i];
        for (int j = 0; j < 64; j++) {
            EXPECT_EQ(data[j], i);
        }
    }

    // Free all
    for (void* ptr : ptrs) {
        hh::basic_alloc::free(ptr);
    }
}

// Test free with nullptr
TEST_F(AllocatorTest, FreeNullptr) {
    void* result = hh::basic_alloc::free(nullptr);
    EXPECT_EQ(result, nullptr);
}

// Test allocation and deallocation pattern
TEST_F(AllocatorTest, AllocFreePairPatterns) {
    void* ptr1 = hh::basic_alloc::try_alloc(100);
    ASSERT_NE(ptr1, nullptr);

    void* ptr2 = hh::basic_alloc::try_alloc(200);
    ASSERT_NE(ptr2, nullptr);

    void* ptr3 = hh::basic_alloc::try_alloc(300);
    ASSERT_NE(ptr3, nullptr);

    // Free middle block
    hh::basic_alloc::free(ptr2);

    // Allocate something that should fit in the freed block
    void* ptr4 = hh::basic_alloc::try_alloc(150);
    ASSERT_NE(ptr4, nullptr);

    hh::basic_alloc::free(ptr1);
    hh::basic_alloc::free(ptr3);
    hh::basic_alloc::free(ptr4);
}

// Test realloc with nullptr (should behave like alloc)
TEST_F(AllocatorTest, ReallocWithNullptr) {
    void* ptr = hh::basic_alloc::try_realloc(nullptr, 100);
    ASSERT_NE(ptr, nullptr);

    memset(ptr, 0xCC, 100);
    unsigned char* data = (unsigned char*)ptr;
    for (int i = 0; i < 100; i++) {
        EXPECT_EQ(data[i], 0xCC);
    }

    hh::basic_alloc::free(ptr);
}

// Test realloc to larger size
TEST_F(AllocatorTest, ReallocLarger) {
    void* ptr = hh::basic_alloc::try_alloc(50);
    ASSERT_NE(ptr, nullptr);

    // Write pattern
    for (int i = 0; i < 50; i++) {
        ((char*)ptr)[i] = i;
    }

    // Realloc to larger size
    void* new_ptr = hh::basic_alloc::try_realloc(ptr, 200);
    ASSERT_NE(new_ptr, nullptr);

    // Verify old data is preserved
    for (int i = 0; i < 50; i++) {
        EXPECT_EQ(((char*)new_ptr)[i], i);
    }

    hh::basic_alloc::free(new_ptr);
}

// Test realloc to smaller size
TEST_F(AllocatorTest, ReallocSmaller) {
    void* ptr = hh::basic_alloc::try_alloc(200);
    ASSERT_NE(ptr, nullptr);

    // Write pattern
    for (int i = 0; i < 200; i++) {
        ((char*)ptr)[i] = i % 256;
    }

    // Realloc to smaller size
    void* new_ptr = hh::basic_alloc::try_realloc(ptr, 50);
    ASSERT_NE(new_ptr, nullptr);

    // Verify first 50 bytes are preserved
    for (int i = 0; i < 50; i++) {
        EXPECT_EQ(((char*)new_ptr)[i], i % 256);
    }

    hh::basic_alloc::free(new_ptr);
}

// Test calloc basic functionality
TEST_F(AllocatorTest, CallocBasic) {
    void* ptr = hh::basic_alloc::try_calloc(10, 10);
    ASSERT_NE(ptr, nullptr);

    // Verify all bytes are zero
    unsigned char* data = (unsigned char*)ptr;
    for (int i = 0; i < 100; i++) {
        EXPECT_EQ(data[i], 0);
    }

    hh::basic_alloc::free(ptr);
}

// Test calloc with zero parameters
TEST_F(AllocatorTest, CallocZeroSize) {
    void* ptr1 = hh::basic_alloc::try_calloc(0, 10);
    EXPECT_EQ(ptr1, nullptr);

    void* ptr2 = hh::basic_alloc::try_calloc(10, 0);
    EXPECT_EQ(ptr2, nullptr);

    void* ptr3 = hh::basic_alloc::try_calloc(0, 0);
    EXPECT_EQ(ptr3, nullptr);
}

// Test calloc overflow protection
TEST_F(AllocatorTest, CallocOverflowProtection) {
    // Try to allocate something that would overflow
    void* ptr = hh::basic_alloc::try_calloc(ULLONG_MAX / 2, ULLONG_MAX / 2);
    EXPECT_EQ(ptr, nullptr);
}

// Test large allocation
TEST_F(AllocatorTest, LargeAllocation) {
    void* ptr = hh::basic_alloc::try_alloc(1024 * 1024);  // 1MB
    ASSERT_NE(ptr, nullptr);

    // Write to some parts of it to make sure it's valid
    ((char*)ptr)[0] = 'A';
    ((char*)ptr)[1024 * 1024 - 1] = 'Z';

    EXPECT_EQ(((char*)ptr)[0], 'A');
    EXPECT_EQ(((char*)ptr)[1024 * 1024 - 1], 'Z');

    hh::basic_alloc::free(ptr);
}

// Test memory coalescing (merging free blocks)
TEST_F(AllocatorTest, MemoryCoalescing) {
    // Allocate three consecutive blocks
    void* ptr1 = hh::basic_alloc::try_alloc(100);
    void* ptr2 = hh::basic_alloc::try_alloc(100);
    void* ptr3 = hh::basic_alloc::try_alloc(100);

    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);
    ASSERT_NE(ptr3, nullptr);

    // Free middle block first
    hh::basic_alloc::free(ptr2);

    // Free first block - should merge with middle
    hh::basic_alloc::free(ptr1);

    // Free last block - should merge with previous merged block
    hh::basic_alloc::free(ptr3);

    // Now allocate a larger block - should reuse merged space
    void* ptr4 = hh::basic_alloc::try_alloc(250);
    ASSERT_NE(ptr4, nullptr);

    hh::basic_alloc::free(ptr4);
}

// Test fragmentation scenario
TEST_F(AllocatorTest, Fragmentation) {
    std::vector<void*> ptrs;

    // Allocate many small blocks
    for (int i = 0; i < 20; i++) {
        void* ptr = hh::basic_alloc::try_alloc(32);
        ASSERT_NE(ptr, nullptr);
        ptrs.push_back(ptr);
    }

    // Free every other block
    for (size_t i = 0; i < ptrs.size(); i += 2) {
        hh::basic_alloc::free(ptrs[i]);
    }

    // Allocate blocks that should fit in freed spaces
    for (int i = 0; i < 10; i++) {
        void* ptr = hh::basic_alloc::try_alloc(24);
        ASSERT_NE(ptr, nullptr);
        // Don't add to ptrs vector, will be cleaned up by later allocations
    }

    // Free remaining original blocks
    for (size_t i = 1; i < ptrs.size(); i += 2) {
        hh::basic_alloc::free(ptrs[i]);
    }
}

// Test allocation patterns with different sizes
TEST_F(AllocatorTest, VariousSizes) {
    size_t sizes[] = {8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
    std::vector<void*> ptrs;

    for (size_t size : sizes) {
        void* ptr = hh::basic_alloc::try_alloc(size);
        ASSERT_NE(ptr, nullptr);

        // Write to allocated memory
        memset(ptr, 0xFF, size);
        ptrs.push_back(ptr);
    }

    // Verify all allocations
    size_t idx = 0;
    for (void* ptr : ptrs) {
        unsigned char* data = (unsigned char*)ptr;
        for (size_t i = 0; i < sizes[idx]; i++) {
            EXPECT_EQ(data[i], 0xFF);
        }
        idx++;
    }

    // Free all
    for (void* ptr : ptrs) {
        hh::basic_alloc::free(ptr);
    }
}

// Test realloc preserves data correctly
TEST_F(AllocatorTest, ReallocPreservesData) {
    void* ptr = hh::basic_alloc::try_alloc(100);
    ASSERT_NE(ptr, nullptr);

    // Write unique pattern
    for (int i = 0; i < 100; i++) {
        ((unsigned char*)ptr)[i] = (i * 7) % 256;
    }

    // Realloc to much larger size (likely new allocation)
    void* new_ptr = hh::basic_alloc::try_realloc(ptr, 1000);
    ASSERT_NE(new_ptr, nullptr);

    // Verify original data is preserved
    for (int i = 0; i < 100; i++) {
        EXPECT_EQ(((unsigned char*)new_ptr)[i], (i * 7) % 256);
    }

    hh::basic_alloc::free(new_ptr);
}

// Test splitting behavior
TEST_F(AllocatorTest, BlockSplitting) {
    // Allocate and free a large block
    void* ptr1 = hh::basic_alloc::try_alloc(1000);
    ASSERT_NE(ptr1, nullptr);
    hh::basic_alloc::free(ptr1);

    // Allocate a much smaller block - should split the large free block
    void* ptr2 = hh::basic_alloc::try_alloc(100);
    ASSERT_NE(ptr2, nullptr);

    // Should be able to allocate another block from the remainder
    void* ptr3 = hh::basic_alloc::try_alloc(100);
    ASSERT_NE(ptr3, nullptr);

    hh::basic_alloc::free(ptr2);
    hh::basic_alloc::free(ptr3);
}

// Test mem_copy helper function
TEST_F(AllocatorTest, MemCopyFunction) {
    char src[50];
    char dest[50];

    // Initialize source with pattern
    for (int i = 0; i < 50; i++) {
        src[i] = i;
    }

    memset(dest, 0, 50);

    hh::basic_alloc::mem_copy(dest, src, 50);

    // Verify copy
    for (int i = 0; i < 50; i++) {
        EXPECT_EQ(dest[i], src[i]);
    }
}

// Test mem_copy with nullptr
TEST_F(AllocatorTest, MemCopyWithNull) {
    char buffer[10];
    memset(buffer, 0, 10);
    // Should handle gracefully
    hh::basic_alloc::mem_copy(nullptr, buffer, 10);
    hh::basic_alloc::mem_copy(buffer, nullptr, 10);
    hh::basic_alloc::mem_copy(buffer, buffer, 0);
}

// Test mem_set helper function
TEST_F(AllocatorTest, MemSetFunction) {
    char buffer[100];

    hh::basic_alloc::mem_set(buffer, 0xAA, 100);

    for (int i = 0; i < 100; i++) {
        EXPECT_EQ((unsigned char)buffer[i], 0xAA);
    }
}

// Test mem_set with nullptr and zero size
TEST_F(AllocatorTest, MemSetWithNullAndZero) {
    hh::basic_alloc::mem_set(nullptr, 0xFF, 100);  // Should not crash

    char buffer[10] = {0};
    hh::basic_alloc::mem_set(buffer, 0xFF, 0);  // Should not modify
    EXPECT_EQ(buffer[0], 0);
}

// Test stress test with many allocations
TEST_F(AllocatorTest, StressTest) {
    std::vector<void*> ptrs;

    // Allocate many blocks of varying sizes
    for (int i = 0; i < 100; i++) {
        size_t size = (i % 10 + 1) * 16;
        void* ptr = hh::basic_alloc::try_alloc(size);
        if (ptr) {
            memset(ptr, i % 256, size);
            ptrs.push_back(ptr);
        }
    }

    EXPECT_GT(ptrs.size(), 90);  // Most should succeed

    // Free half randomly
    std::vector<size_t> indices;
    for (size_t i = 0; i < ptrs.size(); i++) {
    }
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(indices.begin(), indices.end(), g);

    for (size_t i = 0; i < indices.size() / 2; i++) {
        hh::basic_alloc::free(ptrs[indices[i]]);
        ptrs[indices[i]] = nullptr;
    }

    // Allocate more
    for (int i = 0; i < 50; i++) {
        void* ptr = hh::basic_alloc::try_alloc(64);
        if (ptr) {
            ptrs.push_back(ptr);
        }
    }

    // Free all remaining
    for (void* ptr : ptrs) {
        if (ptr) {
            hh::basic_alloc::free(ptr);
        }
    }
}

// Test is_free, make_free, make_used, get_size functions
TEST_F(AllocatorTest, HelperFunctions) {
    hh::basic_alloc::MemSizeT size = 100;

    // Initially not free
    hh::basic_alloc::make_used(size);
    EXPECT_FALSE(hh::basic_alloc::is_free(size));
    EXPECT_EQ(hh::basic_alloc::get_size(size), 100);

    // Mark as free
    hh::basic_alloc::make_free(size);
    EXPECT_TRUE(hh::basic_alloc::is_free(size));
    EXPECT_EQ(hh::basic_alloc::get_size(size), 100);

    // Mark as used again
    hh::basic_alloc::make_used(size);
    EXPECT_FALSE(hh::basic_alloc::is_free(size));
    EXPECT_EQ(hh::basic_alloc::get_size(size), 100);
}

// Test add and sub helper functions
TEST_F(AllocatorTest, AddSubFunctions) {
    hh::basic_alloc::MemSizeT a = 100;
    hh::basic_alloc::MemSizeT b = 50;

    hh::basic_alloc::make_free(a);
    hh::basic_alloc::make_free(b);

    hh::basic_alloc::MemSizeT result_add = hh::basic_alloc::add(a, b);
    EXPECT_EQ(result_add, 150);
    EXPECT_FALSE(hh::basic_alloc::is_free(result_add));  // add should clear free bit

    hh::basic_alloc::MemSizeT result_sub = hh::basic_alloc::sub(a, b);
    EXPECT_EQ(result_sub, 50);
    EXPECT_FALSE(hh::basic_alloc::is_free(result_sub));  // sub should clear free bit
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
