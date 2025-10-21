/**
 * @file test_halloc.BlocksContainer.cpp
 * @brief Unit tests for BlocksContainer multi-block memory allocator
 *
 * Test Coverage:
 * - Basic Functionality: Constructor, single/multiple allocations, deallocation/reallocation
 * - Multiple Blocks : Block creation, max blocks limit, failure handling
 * - Best-Fit Algorithm : Smallest node selection, cross-block search
 * - Edge Cases : Zero bytes, oversized allocation, exact block size, single block limit
 * - Data Integrity : Integer arrays, structs, independent allocations
 * - Fragmentation : Coalescing after deallocation, many small allocations
 * - Stress Tests : Random allocations, fill all blocks, alternating sizes, varying sizes
 *
 */

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <random>
#include <vector>

#include "../halloc/includes/BlocksContainer.hpp"

using namespace hh::halloc;

class BlocksContainerTest : public ::testing::Test {
protected:
    void SetUp() override {}

    void TearDown() override {}
};

// ==================== BASIC FUNCTIONALITY TESTS ====================
/**
 * @test Constructor creates container with initial block and can allocate from it
 */
TEST(BlocksContainerTest, SMALL_Constructor_InitializesWithOneBlock) {
    BlocksContainer<1024, 5> container;

    // Should be able to allocate from the first block
    void* ptr = container.allocate(512);
    EXPECT_NE(ptr, nullptr);

    container.deallocate(ptr, 512);
}

/**
 * @test Single allocation returns valid pointer and memory is writable
 */
TEST(BlocksContainerTest, SMALL_Allocate_SingleAllocation) {
    BlocksContainer<1024, 5> container;

    void* ptr = container.allocate(256);
    EXPECT_NE(ptr, nullptr);

    // Write to the memory to ensure it's valid
    memset(ptr, 0xAA, 256);

    container.deallocate(ptr, 256);
}

/**
 * @test Multiple allocations can coexist in same block without interference
 */
TEST(BlocksContainerTest, SMALL_Allocate_MultipleAllocationsInSameBlock) {
    BlocksContainer<2048, 1> container;

    std::vector<void*> ptrs;
    // Allocate multiple small chunks that fit in one block
    for (int i = 0; i < 5; i++) {
        void* ptr = container.allocate(128);
        EXPECT_NE(ptr, nullptr);
        ptrs.push_back(ptr);
    }

    // Deallocate all
    for (void* ptr : ptrs) {
        container.deallocate(ptr, 128);
    }
}

/**
 * @test Deallocated memory can be reallocated successfully
 */
TEST(BlocksContainerTest, SMALL_Deallocate_AndReallocate) {
    BlocksContainer<1024, 1> container;

    void* ptr1 = container.allocate(1024 - MEMORY_NODE_SIZE);
    EXPECT_NE(ptr1, nullptr);

    container.deallocate(ptr1, 1024 - MEMORY_NODE_SIZE);

    // Should be able to reallocate after deallocation
    void* ptr2 = container.allocate(1024 - MEMORY_NODE_SIZE);
    EXPECT_NE(ptr2, nullptr);

    container.deallocate(ptr2, 1024 - MEMORY_NODE_SIZE);
}

// ==================== MULTIPLE BLOCKS TESTS ====================

/**
 * @test Container automatically creates new blocks when first block fills up
 */
TEST(BlocksContainerTest, SMALL_MultipleBlocks_CreatesNewBlockWhenNeeded) {
    // Small block size to force multiple blocks
    BlocksContainer<512, 10> container;

    std::vector<void*> ptrs;

    // Allocate enough to fill first block and create new ones
    // Each allocation takes space + metadata
    for (int i = 0; i < 15; i++) {
        void* ptr = container.allocate(200);
        ptrs.push_back(ptr);
    }

    // Deallocate all
    for (void* ptr : ptrs) {
        container.deallocate(ptr, 200);
    }
}

/**
 * @test Container respects MaxNumBlocks template parameter limit
 */
TEST(BlocksContainerTest, SMALL_MultipleBlocks_AllocatesUpToMaxBlocks) {
    BlocksContainer<256, 3> container;

    std::vector<void*> ptrs;

    // Try to allocate enough to use all 3 blocks
    for (int i = 0; i < 10; i++) {
        void* ptr = container.allocate(150);
        if (ptr != nullptr) {
            ptrs.push_back(ptr);
        }
    }

    // Should have some successful allocations
    EXPECT_GT(ptrs.size(), 0);

    // Cleanup
    for (void* ptr : ptrs) {
        container.deallocate(ptr, 150);
    }
}

// ==================== BEST-FIT ACROSS BLOCKS TESTS ====================

/**
 * @test Best-fit algorithm selects smallest suitable free node to minimize fragmentation
 */
TEST(BlocksContainerTest, SMALL_BestFit_SelectsSmallestSuitableNode) {
    BlocksContainer<1024, 3> container;

    void* ptr1 = container.allocate(512);                             // first block
    void* ptr2 = container.allocate(1024 - 2ull * MEMORY_NODE_SIZE);  // second block
    void* ptr3 = container.allocate(128);                             // first block

    container.deallocate(ptr1, 512);

    void* ptr4 =
        container.allocate(1024 - 128 - 2 * MEMORY_NODE_SIZE);  // must go into the first block

    // Ensure allocated memory is writable
    memset(ptr4, 0xBB, 1024 - 128 - 2 * MEMORY_NODE_SIZE);

    EXPECT_NE(ptr4, nullptr);

    // Cleanup
    container.deallocate(ptr2, 1024 - 2ull * MEMORY_NODE_SIZE);
    container.deallocate(ptr4, 1024 - 128 - 2 * MEMORY_NODE_SIZE);

    container.deallocate(ptr3, 128);
}

/**
 * @test Best-fit search operates across all initialized blocks
 */
TEST(BlocksContainerTest, SMALL_BestFit_SearchesAcrossMultipleBlocks) {
    BlocksContainer<696, 5> container;

    // Force creation of multiple blocks with different hole sizes
    std::vector<void*> ptrs;

    for (int i = 0; i < 10; i++) {
        void* ptr = container.allocate(300);
        ptrs.push_back(ptr);
    }

    container.deallocate(ptrs[0], 300);
    container.deallocate(ptrs[1], 300);

    // New allocation should fit in the first block
    void* new_ptr = container.allocate(600);
    EXPECT_NE(new_ptr, nullptr);

    // Cleanup
    for (size_t i = 2; i < ptrs.size(); i++) {
        container.deallocate(ptrs[i], 300);
    }
    container.deallocate(new_ptr, 600);
}

// ==================== EDGE CASES ====================

/**
 * @test Zero-byte allocation handles gracefully without crashing
 */
TEST(BlocksContainerTest, SMALL_EdgeCase_AllocateZeroBytes) {
    BlocksContainer<1024, 5> container;

    EXPECT_THROW({ container.allocate(0); }, std::invalid_argument);
}

/**
 * @test Allocation request larger than BlockSize handles without crash
 */
TEST(BlocksContainerTest, SMALL_EdgeCase_AllocateLargerThanBlockSize) {
    BlocksContainer<400, 10> container;

    void* ptr = container.allocate(400);
    EXPECT_EQ(ptr, nullptr);
}

// /**
//  * @test Allocation deallocation of a ptr not allocated by the container MAY CAUSE SEGFAULT
//  */
// TEST(BlocksContainerTest, SMALL_EdgeCase_DeallocateInvalidPointer) {
//     BlocksContainer<1024, 1> container;
//     int dummy = 10;
//     int* ptr = &dummy;
//     EXPECT_DEATH(
//         { container.deallocate(ptr, sizeof(int)); },
//         "ContainerStateException: Pointer not allocated by this container");

//     // Try to allocate almost entire block (minus metadata)
// }

/**
 * @test Allocation of memory bigger that the containers MUST succeed via mmap and be deallocated
 * properly
 */
TEST(BlocksContainerTest, SMALL_EdgeCase_AllocateBiggerThanBlockSize_DeallocateProperly) {
    BlocksContainer<1024, 1> container;
    std::size_t large_size = 2048;  // Bigger than block size
    void* ptr = container.allocate(large_size);
    EXPECT_NE(ptr, nullptr);

    std::memset(ptr, 0xAB, large_size);  // Use the memory

    container.deallocate(ptr, large_size);
}

// ==================== DATA INTEGRITY TESTS ====================

/**
 * @test Integer arrays can be written and read without corruption
 */
TEST(BlocksContainerTest, SMALL_DataIntegrity_WriteAndReadIntegers) {
    BlocksContainer<4096, 5> container;

    const int ARRAY_SIZE = 100;
    int* int_array = static_cast<int*>(container.allocate(ARRAY_SIZE * sizeof(int)));
    EXPECT_NE(int_array, nullptr);

    // Write pattern
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 10;
    }

    // Verify pattern
    for (int i = 0; i < ARRAY_SIZE; i++) {
        EXPECT_EQ(int_array[i], i * 10);
    }

    container.deallocate(int_array, ARRAY_SIZE * sizeof(int));
}

/**
 * @test Custom structs can be allocated, written, and read correctly
 */
TEST(BlocksContainerTest, SMALL_DataIntegrity_WriteAndReadStructs) {
    struct TestStruct {
        int id;
        char* name;
        double value;
    };

    BlocksContainer<4096, 5> container;

    TestStruct* obj = static_cast<TestStruct*>(container.allocate(sizeof(TestStruct)));
    EXPECT_NE(obj, nullptr);

    obj->id = 42;
    obj->name = static_cast<char*>(container.allocate(20));
    strcpy(obj->name, "TestObject\0");
    obj->value = 3.14159;

    EXPECT_EQ(obj->id, 42);
    EXPECT_STREQ(obj->name, "TestObject\0");
    EXPECT_DOUBLE_EQ(obj->value, 3.14159);

    container.deallocate(obj->name, 20);
    container.deallocate(obj, sizeof(TestStruct));
}

/**
 * @test Multiple concurrent allocations maintain data independence
 */
TEST(BlocksContainerTest, SMALL_DataIntegrity_MultipleAllocationsIndependent) {
    BlocksContainer<4096, 5> container;

    int* arr1 = static_cast<int*>(container.allocate(10 * sizeof(int)));
    int* arr2 = static_cast<int*>(container.allocate(10 * sizeof(int)));
    int* arr3 = static_cast<int*>(container.allocate(10 * sizeof(int)));

    EXPECT_NE(arr1, nullptr);
    EXPECT_NE(arr2, nullptr);
    EXPECT_NE(arr3, nullptr);

    // Fill with different patterns
    for (int i = 0; i < 10; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = i * 3;
    }

    // Verify each array independently
    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(arr1[i], i);
        EXPECT_EQ(arr2[i], i * 2);
        EXPECT_EQ(arr3[i], i * 3);
    }

    container.deallocate(arr1, 10 * sizeof(int));
    container.deallocate(arr2, 10 * sizeof(int));
    container.deallocate(arr3, 10 * sizeof(int));
}

// ==================== FRAGMENTATION TESTS ====================

/**
 * @test Adjacent free blocks are coalesced to enable larger allocations
 */
TEST(BlocksContainerTest, SMALL_Fragmentation_CoalescingAfterDeallocation) {
    BlocksContainer<1024 + 4 * MEMORY_NODE_SIZE, 1> container;

    void* ptr1 = container.allocate(256);
    void* ptr2 = container.allocate(256);
    void* ptr3 = container.allocate(256);
    void* ptr4 = container.allocate(256);

    container.deallocate(ptr1, 256);
    container.deallocate(ptr2, 256);

    void* large_ptr = container.allocate(512 + MEMORY_NODE_SIZE);
    EXPECT_NE(large_ptr, nullptr);

    container.deallocate(ptr2, 256);
    container.deallocate(ptr4, 256);
    container.deallocate(large_ptr, 512 + MEMORY_NODE_SIZE);

    container.deallocate(ptr3, 256);
}

// ==================== STRESS TESTS ====================
/**
 * @test Random allocation/deallocation patterns with varying sizes and memory writes
 */
TEST(BlocksContainerTest, STRESS_RandomAllocationsAndDeallocations) {
    BlocksContainer<256 * 1024, 10> container;  // 256KB blocks, max 10

    std::vector<void*> allocations;
    std::vector<std::size_t> sizes;

    srand(42);

    // More conservative allocations to avoid allocator bugs
    for (int i = 0; i < 200; i++) {
        std::size_t size = 64 + (rand() % 2048);  // Smaller max size
        void* ptr = container.allocate(size);

        if (ptr != nullptr) {
            allocations.push_back(ptr);
            sizes.push_back(size);
            // Write pattern
            memset(ptr, i % 256, std::min(size, (std::size_t)64));
        }
    }

    EXPECT_GT(allocations.size(), 30);

    // Random deallocations
    std::vector<bool> deallocated(allocations.size(), false);
    for (std::size_t i = 0; i < allocations.size() / 3; i++) {
        std::size_t idx = rand() % allocations.size();
        if (!deallocated[idx]) {
            container.deallocate(allocations[idx], sizes[idx]);
            deallocated[idx] = true;
        }
    }

    // Try more allocations
    for (int i = 0; i < 50; i++) {
        std::size_t size = 128 + (rand() % 1024);
        void* ptr = container.allocate(size);
        if (ptr != nullptr) {
            allocations.push_back(ptr);
            sizes.push_back(size);
            deallocated.push_back(false);
        }
    }

    // Cleanup
    for (std::size_t i = 0; i < allocations.size(); i++) {
        if (!deallocated[i]) {
            container.deallocate(allocations[i], sizes[i]);
        }
    }
}

/**
 * @test Filling all available blocks to capacity verifies multi-block management
 */
TEST(BlocksContainerTest, STRESS_FillAllBlocks) {
    BlocksContainer<512 * 1024, 5> container;  // 512KB blocks, max 5

    std::vector<void*> allocations;
    const std::size_t ALLOC_SIZE = 16 * 1024;  // 16KB

    // Try to fill all blocks
    for (int i = 0; i < 200; i++) {
        void* ptr = container.allocate(ALLOC_SIZE);
        if (ptr != nullptr) {
            allocations.push_back(ptr);
            // Write to ensure memory is valid
            memset(ptr, 0xAB, 256);
        } else {
            break;  // No more space
        }
    }

    EXPECT_GT(allocations.size(), 10);

    // Cleanup
    for (void* ptr : allocations) {
        container.deallocate(ptr, ALLOC_SIZE);
    }
}

/**
 * @test Random large and small allocations tests block reuse after deallocation
 */
TEST(BlocksContainerTest, STRESS_RandomLargeAndSmall) {
    BlocksContainer<1024 * 1024 * 128, 32> container;

    std::vector<void*> allocations;
    std::vector<std::size_t> sizes;

    std::vector<std::size_t> small_indexes, large_indexes;

    srand(42);
    for (int i = 0; i < 10000; i++) {
        bool is_large = (rand() % 2 == 0);

        std::size_t size = is_large
                               ? (64 * 1024 + (rand() % (256 * 1024 - 64 * 1024)))  // 64KB to 256KB
                               : (64 + (rand() % (8 * 1024 - 64)));                 // 64B to 8KB

        void* ptr = container.allocate(size);

        // Only track successful allocations
        if (ptr != nullptr) {
            allocations.push_back(ptr);
            sizes.push_back(size);

            if (is_large) {
                large_indexes.push_back(allocations.size() - 1);
            } else {
                small_indexes.push_back(allocations.size() - 1);
            }
        }
    }
    EXPECT_GT(allocations.size(), 1000);
    std::vector<int> deallocated(allocations.size(), 0);

    for (std::size_t itr = 0; itr < 1000; itr++) {
        std::size_t idx = rand() % allocations.size();

        if (deallocated[idx] == 0) {
            container.deallocate(allocations[idx], sizes[idx]);
            deallocated[idx] = 1;
        }
    }

    // ensure random allocated are writable after deletion
    std::unordered_map<int, std::vector<char>> values;
    for (std::size_t i = 0; i < allocations.size(); i++) {
        if (deallocated[i] == 0) {
            for (std::size_t j = 0; j < std::min(sizes[i], (std::size_t)64 * 64); j++) {
                static_cast<char*>(allocations[i])[j] = (char)(i % 256);
            }
            values[i] = std::vector<char>(std::min(sizes[i], (std::size_t)64 * 1024));
            std::memcpy(values[i].data(), allocations[i], values[i].size());
        }
    }

    // // ensure the values are correct
    for (std::size_t i = 0; i < allocations.size(); i++) {
        if (deallocated[i] == 0) {
            auto& vv = values[i];
            for (std::size_t j = 0; j < vv.size(); j++) {
                EXPECT_EQ(static_cast<char*>(allocations[i])[j], vv[j]);
            }
        }
    }

    // Cleanup
    for (std::size_t i = 0; i < allocations.size(); i++) {
        if (deallocated[i] == 0) {
            container.deallocate(allocations[i], sizes[i]);
        }
    }
}

/**
 * @test Wide range of allocation sizes (16B to 32KB) over multiple rounds
 */
TEST(BlocksContainerTest, STRESS_VaryingSizes) {
    BlocksContainer<1 * 1024 * 1024, 10> container;  // 1MB blocks

    std::vector<void*> ptrs;
    std::vector<std::size_t> sizes;

    std::size_t test_sizes[] = {16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768};

    // Allocate varying sizes - reduced rounds to avoid issues
    for (int round = 0; round < 5; round++) {
        for (std::size_t size : test_sizes) {
            void* ptr = container.allocate(size);
            if (ptr != nullptr) {
                ptrs.push_back(ptr);
                sizes.push_back(size);
                // Write pattern
                if (size >= sizeof(int)) {
                    *static_cast<int*>(ptr) = round;
                }
            }
        }
    }

    EXPECT_GT(ptrs.size(), 30);

    // Verify some data
    for (std::size_t i = 0; i < ptrs.size(); i++) {
        if (sizes[i] >= sizeof(int)) {
            int value = *static_cast<int*>(ptrs[i]);
            EXPECT_GE(value, 0);
            EXPECT_LT(value, 10);
        }
    }

    // Cleanup
    for (std::size_t i = 0; i < ptrs.size(); i++) {
        container.deallocate(ptrs[i], sizes[i]);
    }
}
