/**
 * @file test_basic_alloc.cpp
 * @brief  Unit tests for basic linked-list memory allocator.
 *
 * Test Coverage:
 * - Utility Functions: mem_copy, mem_set, size manipulation
 * - Basic Allocations: try_alloc, try_free, try_realloc, try_calloc
 * - Edge Cases: zero-size allocs, realloc with nullptr, free nullptr
 * - Advanced Scenarios: fragmentation, coalescing, stress tests
 *
 */

#include <gtest/gtest.h>

#include <algorithm>
#include <cstring>
#include <random>
#include <vector>

#include "../basic-allocator/basic_alloc.hpp"

class BasicAllocatorTest : public ::testing::Test {
protected:
    void SetUp() override {}

    void TearDown() override {}
};

// ================== Utility Function Tests ==================
TEST(BasicAllocatorTest, SMALL_MemCopyFunction) {
    char src[50];
    char dest[50];

    for (int i = 0; i < 50; i++) {
        src[i] = i;
    }

    memset(dest, 0, 50);

    hh::basic_alloc::mem_copy(dest, src, 50);

    for (int i = 0; i < 50; i++) {
        EXPECT_EQ(dest[i], src[i]);
    }
}

TEST(BasicAllocatorTest, SMALL_MemSetFunction) {
    char buffer[100];

    hh::basic_alloc::mem_set(buffer, 0xAA, 100);

    for (int i = 0; i < 100; i++) {
        EXPECT_EQ((unsigned char)buffer[i], 0xAA);
    }
}

TEST(BasicAllocatorTest, SMALL_MemSetWithNullAndZero) {
    hh::basic_alloc::mem_set(nullptr, 0xFF, 100);  // Should not crash

    char buffer[10] = {0};
    hh::basic_alloc::mem_set(buffer, 0xFF, 0);  // Should not modify
    EXPECT_EQ(buffer[0], 0);
}

TEST(BasicAllocatorTest, SMALL_HelperFunctions) {
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

TEST(BasicAllocatorTest, SMALL_AddSubFunctions) {
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

// ================== Allocation Tests ==================
TEST(BasicAllocatorTest, SMALL_BasicAllocation) {
    void* ptr = hh::basic_alloc::try_alloc(100);
    ASSERT_NE(ptr, nullptr);

    memset(ptr, 0xAB, 100);

    unsigned char* data = (unsigned char*)ptr;
    for (int i = 0; i < 100; i++) {
        EXPECT_EQ(data[i], 0xAB);
    }

    hh::basic_alloc::free(ptr);
}

TEST(BasicAllocatorTest, SMALL_ZeroSizeAllocation) {
    void* ptr = hh::basic_alloc::try_alloc(0);
    EXPECT_EQ(ptr, nullptr);
}

TEST(BasicAllocatorTest, SMALL_MultipleAllocations) {
    std::vector<void*> ptrs;

    for (int i = 0; i < 10; i++) {
        void* ptr = hh::basic_alloc::try_alloc(64);
        ASSERT_NE(ptr, nullptr);
        ptrs.push_back(ptr);

        memset(ptr, i, 64);
    }

    for (size_t i = 0; i < ptrs.size(); i++) {
        unsigned char* data = (unsigned char*)ptrs[i];
        for (int j = 0; j < 64; j++) {
            EXPECT_EQ(data[j], i);
        }
    }

    for (void* ptr : ptrs) {
        hh::basic_alloc::free(ptr);
    }
}

TEST(BasicAllocatorTest, SMALL_FreeNullptr) {
    void* result = hh::basic_alloc::free(nullptr);
    EXPECT_EQ(result, nullptr);
}

TEST(BasicAllocatorTest, SMALL_AllocFreePairPatterns) {
    void* ptr1 = hh::basic_alloc::try_alloc(100);
    ASSERT_NE(ptr1, nullptr);

    void* ptr2 = hh::basic_alloc::try_alloc(200);
    ASSERT_NE(ptr2, nullptr);

    void* ptr3 = hh::basic_alloc::try_alloc(300);
    ASSERT_NE(ptr3, nullptr);

    hh::basic_alloc::free(ptr2);

    void* ptr4 = hh::basic_alloc::try_alloc(150);
    ASSERT_NE(ptr4, nullptr);

    hh::basic_alloc::free(ptr1);
    hh::basic_alloc::free(ptr3);
    hh::basic_alloc::free(ptr4);
}
// ================== Realloc Tests ==================

TEST(BasicAllocatorTest, SMALL_ReallocWithNullptr) {
    void* ptr = hh::basic_alloc::try_realloc(nullptr, 100);
    ASSERT_NE(ptr, nullptr);

    memset(ptr, 0xCC, 100);
    unsigned char* data = (unsigned char*)ptr;
    for (int i = 0; i < 100; i++) {
        EXPECT_EQ(data[i], 0xCC);
    }

    hh::basic_alloc::free(ptr);
}

TEST(BasicAllocatorTest, SMALL_ReallocLarger) {
    void* ptr = hh::basic_alloc::try_alloc(50);
    ASSERT_NE(ptr, nullptr);

    for (int i = 0; i < 50; i++) {
        ((char*)ptr)[i] = i;
    }

    void* new_ptr = hh::basic_alloc::try_realloc(ptr, 200);
    ASSERT_NE(new_ptr, nullptr);

    for (int i = 0; i < 50; i++) {
        EXPECT_EQ(((char*)new_ptr)[i], i);
    }

    hh::basic_alloc::free(new_ptr);
}

// ================== Calloc Tests ==================
TEST(BasicAllocatorTest, SMALL_CallocBasic) {
    void* ptr = hh::basic_alloc::try_calloc(10, 10);
    ASSERT_NE(ptr, nullptr);

    // Verify all bytes are zero
    unsigned char* data = (unsigned char*)ptr;
    for (int i = 0; i < 100; i++) {
        EXPECT_EQ(data[i], 0);
    }

    hh::basic_alloc::free(ptr);
}

TEST(BasicAllocatorTest, SMALL_CallocZeroSize) {
    void* ptr1 = hh::basic_alloc::try_calloc(0, 10);
    EXPECT_EQ(ptr1, nullptr);

    void* ptr2 = hh::basic_alloc::try_calloc(10, 0);
    EXPECT_EQ(ptr2, nullptr);

    void* ptr3 = hh::basic_alloc::try_calloc(0, 0);
    EXPECT_EQ(ptr3, nullptr);
}

TEST(BasicAllocatorTest, CallocOverflowProtection) {
    void* ptr = hh::basic_alloc::try_calloc(ULLONG_MAX / 2, ULLONG_MAX / 2);
    EXPECT_EQ(ptr, nullptr);
}

// ================== Advanced Tests ==================
TEST(BasicAllocatorTest, SMALL_MemoryCoalescing) {
    void* ptr1 = hh::basic_alloc::try_alloc(100);
    void* ptr2 = hh::basic_alloc::try_alloc(100);
    void* ptr3 = hh::basic_alloc::try_alloc(100);

    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);
    ASSERT_NE(ptr3, nullptr);

    hh::basic_alloc::free(ptr2);

    hh::basic_alloc::free(ptr1);

    hh::basic_alloc::free(ptr3);

    void* ptr4 = hh::basic_alloc::try_alloc(250);
    ASSERT_NE(ptr4, nullptr);

    hh::basic_alloc::free(ptr4);
}

TEST(BasicAllocatorTest, SMALL_Fragmentation) {
    std::vector<void*> ptrs;

    for (int i = 0; i < 20; i++) {
        void* ptr = hh::basic_alloc::try_alloc(32);
        ASSERT_NE(ptr, nullptr);
        ptrs.push_back(ptr);
    }

    for (size_t i = 0; i < ptrs.size(); i += 2) {
        hh::basic_alloc::free(ptrs[i]);
    }

    for (int i = 0; i < 10; i++) {
        void* ptr = hh::basic_alloc::try_alloc(24);
        ASSERT_NE(ptr, nullptr);
    }

    for (size_t i = 1; i < ptrs.size(); i += 2) {
        hh::basic_alloc::free(ptrs[i]);
    }
}

TEST(BasicAllocatorTest, SMALL_ReallocPreservesData) {
    void* ptr = hh::basic_alloc::try_alloc(100);
    ASSERT_NE(ptr, nullptr);

    for (int i = 0; i < 100; i++) {
        ((unsigned char*)ptr)[i] = (i * 7) % 256;
    }

    void* new_ptr = hh::basic_alloc::try_realloc(ptr, 1000);
    ASSERT_NE(new_ptr, nullptr);

    for (int i = 0; i < 100; i++) {
        EXPECT_EQ(((unsigned char*)new_ptr)[i], (i * 7) % 256);
    }

    hh::basic_alloc::free(new_ptr);
}

TEST(BasicAllocatorTest, SMALL_BlockSplitting) {
    void* ptr1 = hh::basic_alloc::try_alloc(1000);
    ASSERT_NE(ptr1, nullptr);
    hh::basic_alloc::free(ptr1);

    void* ptr2 = hh::basic_alloc::try_alloc(100);
    ASSERT_NE(ptr2, nullptr);

    void* ptr3 = hh::basic_alloc::try_alloc(100);
    ASSERT_NE(ptr3, nullptr);

    hh::basic_alloc::free(ptr2);
    hh::basic_alloc::free(ptr3);
}

// ================= Stress Tests =================
TEST(BasicAllocatorTest, STRESS_StressTestAllocFree) {
    const int NUM_OPERATIONS = 20000;
    const int MIN_ALLOC_SIZE = 64;
    const int MAX_ALLOC_SIZE = 1024 * 64;

    std::vector<void*> allocations(NUM_OPERATIONS, nullptr);
    std::default_random_engine generator;
    std::uniform_int_distribution<int> size_distribution(MIN_ALLOC_SIZE, MAX_ALLOC_SIZE);
    std::uniform_int_distribution<int> operation_distribution(0, 5);  // 0 = free, other = alloc

    for (int i = 0; i < NUM_OPERATIONS; i++) {
        int operation = operation_distribution(generator);

        if (operation) {
            int size = size_distribution(generator);
            void* ptr = hh::basic_alloc::try_alloc((size));  // align to 4 bytes
            if (ptr != nullptr) {
                allocations[i] = ptr;
                hh::basic_alloc::mem_set(ptr, 1, size);
            }
        } else {
            // Free
            int index = std::uniform_int_distribution<int>(0, i)(generator);
            if (allocations[index] != nullptr) {
                allocations[index] = hh::basic_alloc::free(allocations[index]);
            }
        }
    }

    for (void* ptr : allocations) {
        if (ptr) {
            EXPECT_EQ(*static_cast<unsigned char*>((unsigned char*)ptr + 63), 1);
        }
    }

    for (void* ptr : allocations) {
        if (ptr)
            hh::basic_alloc::free(ptr);
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
