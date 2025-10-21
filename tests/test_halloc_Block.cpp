/**
 * @file test_halloc_Block.cpp
 * @brief Unit tests for Block single-block memory allocator with RB-tree
 *
 * Test Coverage:
 * - Basic Allocation : Full block, smaller sizes, arrays, structs, multiple allocations
 * - Memory Management: Block metadata verification, coalescing on deallocation
 * - Stress Tests : Random patterns (50K allocs), fragmentation, RB-tree depth
 *
 */

#include <gtest/gtest.h>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <algorithm>

#include "../halloc/includes/Block.hpp"

using namespace hh::halloc;

namespace
{
    std::size_t get_actual_value(std::size_t value)
    {
        return value & ~(3ull << 62);
    }
}

class HallocBlockTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

void *allocate(Block &block, std::size_t size)
{
    MemoryNode *node1 = block.best_fit(size);
    if (!node1)
        return nullptr;

    void *ptr1 = block.allocate(size, node1);

    return ptr1;
}

/**
 * @test Allocating entire block size works, prevents further allocations, and allows reallocation after deallocation
 */
TEST(HallocBlockTest, SMALL_AllocateTheSameSizeAsBlock)
{
    Block block(1024);

    MemoryNode *node = block.best_fit(1024 - 48);
    EXPECT_NE(node, nullptr);

    void *ptr = block.allocate(1024 - 48, node);
    EXPECT_NE(ptr, nullptr);

    MemoryNode *node_after_allocation = block.best_fit(128);
    EXPECT_EQ(node_after_allocation, nullptr);

    block.deallocate(ptr, 1024 - 48);

    MemoryNode *node_after_deallocation = block.best_fit(512);
    EXPECT_NE(node_after_deallocation, nullptr);

    void *ptr2 = block.allocate(512 - 48, node_after_deallocation);
    EXPECT_NE(ptr2, nullptr);

    MemoryNode *node_after_second_allocation = block.best_fit(512);
    // Must Be nullptr as the node size has used
    EXPECT_EQ(node_after_second_allocation, nullptr);
}

/**
 * @test Small allocations with splitting and coalescing verify block reuse efficiency
 */
TEST(HallocBlockTest, SMALL_AllocateSmallerSizes)
{
    // first 48 is for MEMORY_NODE_SIZE
    Block block(100);

    void *ptr1 = allocate(block, 100 - 48);
    EXPECT_NE(ptr1, nullptr);

    // No more space left
    void *ptr2 = allocate(block, 4);
    EXPECT_EQ(ptr2, nullptr);

    // second param is unused
    block.deallocate(ptr1, std::numeric_limits<std::size_t>::max());

    // Now we can allocate again (this actually takes 50 bytes)
    void *ptr3 = allocate(block, 2);
    EXPECT_NE(ptr3, nullptr);

    // Also we can allocate another 2 bytes (50 + 50 = 100)
    void *ptr4 = allocate(block, 2);
    EXPECT_NE(ptr4, nullptr);

    block.deallocate(ptr3, std::numeric_limits<std::size_t>::max());
    block.deallocate(ptr4, std::numeric_limits<std::size_t>::max());

    // Now we can allocate full block again
    void *ptr5 = allocate(block, 100 - 48);
    EXPECT_NE(ptr5, nullptr);
}

/**
 * @test Integer array allocation with write/read verification ensures data integrity
 */
TEST(HallocBlockTest, SMALL_AllocateAndUseINTArray)
{
    const int ARRAY_SIZE = 10;
    Block block(10 * sizeof(int) + MEMORY_NODE_SIZE);

    int *int_array = static_cast<int *>(allocate(block, ARRAY_SIZE * sizeof(int)));
    EXPECT_NE(int_array, nullptr);

    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        int_array[i] = i * 10;
    }

    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        EXPECT_EQ(int_array[i], i * 10);
    }

    block.deallocate(int_array, ARRAY_SIZE * sizeof(int));
}

/**
 * @test Custom struct with nested allocation verifies complex object support and best-fit behavior
 */
TEST(HallocBlockTest, SMALL_AllocateAndUseCustomStruct)
{

    struct CS
    {
        int id;
        char *data;
        long long value;
    };

    Block block(11 + sizeof(CS) + MEMORY_NODE_SIZE + MEMORY_NODE_SIZE);
    EXPECT_EQ(((MemoryNode *)block.get_head())->value, 11 + sizeof(CS) + MEMORY_NODE_SIZE);

    auto best = block.best_fit(sizeof(CS));
    EXPECT_EQ(best, block.get_head());
    CS *cs_ptr = static_cast<CS *>(block.allocate(sizeof(CS), best));

    EXPECT_EQ(best, block.get_head());

    cs_ptr->data = static_cast<char *>(allocate(block, 11));
    EXPECT_NE(cs_ptr->data, nullptr);

    cs_ptr->data[10] = '\0';
    for (int i = 0; i < 10; i++)
    {
        cs_ptr->data[i] = 'A' + i;
    }

    cs_ptr->id = 42;
    cs_ptr->value = 1234567890LL;

    EXPECT_EQ(cs_ptr->id, 42);
    EXPECT_EQ(cs_ptr->value, 1234567890LL);

    // block.print_tree_info();
    block.deallocate(cs_ptr, sizeof(CS));
    block.deallocate(cs_ptr->data, 11);
}

/**
 * @test Sequential allocations of varying sizes verify metadata tracking and size correctness
 */
TEST(HallocBlockTest, SMALL_MultipleAllocations)
{
    Block block(2048);
    std::vector<int> vals = {16, 32, 64, 128, 256, 512};
    std::vector<void *> mem;
    for (std::size_t i = 0; i < vals.size(); i++)
    {
        void *ptr = allocate(block, vals[i]);
        EXPECT_NE(ptr, nullptr);
        mem.push_back(ptr);
    }

    for (int i = vals.size() - 1; i >= 0; i--)
    {
        EXPECT_EQ(get_actual_value(((MemoryNode *)((unsigned char *)mem[i] - MEMORY_NODE_SIZE))->value), vals[i]);
    }
}

/**
 * @test Deallocations create merged free blocks enabling larger allocations (coalescing verification)
 */
TEST(HallocBlockTest, SMALL_MultipleAllocationsWithDeletionsMustMerge)
{
    Block block(1311);
    std::vector<int> vals = {16, 32, 64, 128, 256, 512};
    std::vector<void *> mem;
    for (std::size_t i = 0; i < vals.size(); i++)
    {
        void *ptr = allocate(block, vals[i]);
        mem.push_back(ptr);
    }

    block.deallocate(mem[3], std::numeric_limits<std::size_t>::max());
    block.deallocate(mem[4], std::numeric_limits<std::size_t>::max());

    int *ptr = (int *)allocate(block, 432);
    EXPECT_NE(ptr, nullptr);

    int *ptr2 = (int *)allocate(block, 4);
    EXPECT_EQ(ptr2, nullptr);
}

/**
 * @test 50K random allocations with 60% deallocation followed by 10K reallocations tests memory reuse
 */
TEST(HallocBlockTest, STRESS_RandomAllocationsAndDeallocations)
{
    Block block(512 * 1024 * 1024); // 512MB block

    std::vector<void *> allocations;
    std::vector<std::size_t> sizes;

    srand(42);

    for (int i = 0; i < 50000; i++)
    {
        // Random size between 64 and 16KB
        std::size_t size = 64 + (rand() % (16 * 1024 - 64));
        char *ptr = (char *)allocate(block, size);

        EXPECT_NE(ptr, nullptr);

        allocations.push_back(ptr);
        sizes.push_back(size);
        for (std::size_t j = 0; j < size; j++)
        {
            ptr[j] = 'A' + (i % 26);
        }
    }
    std::vector<bool> deallocated(allocations.size(), false);
    for (std::size_t i = 0; i < allocations.size() * 6 / 10; i++)
    {
        std::size_t idx = rand() % allocations.size();
        if (!deallocated[idx])
        {
            block.deallocate(allocations[idx], sizes[idx]);
            deallocated[idx] = true;
        }
    }

    int reused_count = 0;
    for (int i = 0; i < 10000; i++)
    {
        std::size_t size = 128 + (rand() % (8 * 1024));
        void *ptr = allocate(block, size);
        if (ptr != nullptr)
        {
            reused_count++;
        }
    }

    EXPECT_GT(reused_count, 3000);

    for (std::size_t i = 0; i < allocations.size(); i++)
    {
        if (!deallocated[i])
        {
            block.deallocate(allocations[i], sizes[i]);
        }
    }
}

/**
 * @test Alternating allocation/deallocation patterns verify coalescing under heavy fragmentation (10K+ allocs)
 */
TEST(HallocBlockTest, STRESS_FragmentationAndCoalescing)
{
    Block block(1024 * 1024 * 1024);

    std::vector<void *> allocations;
    const int NUM_ALLOCATIONS = 10000;
    const std::size_t ALLOC_SIZE = 64 * 1024;

    for (int i = 0; i < NUM_ALLOCATIONS; i++)
    {
        void *ptr = allocate(block, ALLOC_SIZE);
        if (ptr != nullptr)
        {
            allocations.push_back(ptr);
            for (std::size_t j = 0; j < std::min(ALLOC_SIZE, (std::size_t)4096); j += sizeof(int))
            {
                *((int *)((char *)ptr + j)) = i;
            }
        }
    }

    EXPECT_GT(allocations.size(), 5000);

    std::vector<void *> kept_allocations;
    for (std::size_t i = 0; i < allocations.size(); i++)
    {
        if (i % 2 == 0)
        {
            block.deallocate(allocations[i], ALLOC_SIZE);
        }
        else
        {
            kept_allocations.push_back(allocations[i]);
        }
    }

    std::vector<void *> large_allocations;
    for (std::size_t i = 0; i < 1000; i++)
    {
        void *ptr = allocate(block, ALLOC_SIZE * 2);
        if (ptr != nullptr)
        {
            large_allocations.push_back(ptr);
        }
    }

    for (void *ptr : kept_allocations)
    {
        block.deallocate(ptr, ALLOC_SIZE);
    }

    std::vector<void *> huge_allocations;
    for (int i = 0; i < 20; i++)
    {
        void *huge_ptr = allocate(block, 10 * 1024 * 1024);
        if (huge_ptr != nullptr)
        {
            huge_allocations.push_back(huge_ptr);
        }
    }

    EXPECT_GT(huge_allocations.size(), 10);

    // Cleanup
    for (void *ptr : large_allocations)
    {
        block.deallocate(ptr, ALLOC_SIZE * 2);
    }
    for (void *ptr : huge_allocations)
    {
        block.deallocate(ptr, 10 * 1024 * 1024);
    }
}

/**
 * @test Exponentially increasing sizes (1KB to GB) with random deallocation stresses RB-tree balancing (100K+ ops)
 */
TEST(HallocBlockTest, STRESS_WorstCaseRBTreeDepth)
{
    Block block(2ULL * 1024 * 1024 * 1024); // 2GB block

    std::vector<void *> allocations;
    std::vector<std::size_t> sizes;

    std::size_t base_size = 1024; // Start with 1KB
    for (std::size_t i = 0; i < 25; i++)
    {
        std::size_t size = base_size * (1ULL << i); // 1KB, 2KB, 4KB, 8KB, ..., up to 16GB (but limited by block)

        void *ptr = allocate(block, size);
        if (ptr != nullptr)
        {
            allocations.push_back(ptr);
            sizes.push_back(size);
        }
    }

    EXPECT_GT(allocations.size(), 15);

    std::vector<std::size_t> indices;
    for (std::size_t i = 0; i < allocations.size(); i++)
    {
        indices.push_back(i);
    }

    srand(12345);
    for (std::size_t i = indices.size() - 1; i > 0; i--)
    {
        std::size_t j = rand() % (i + 1);
        std::swap(indices[i], indices[j]);
    }

    for (std::size_t idx : indices)
    {
        block.deallocate(allocations[idx], sizes[idx]);
    }

    std::vector<void *> small_allocations;
    std::vector<std::size_t> small_sizes;

    for (int i = 0; i < 100000; i++)
    {
        std::size_t size = 1024 + (i % 128) * 1024;
        void *ptr = allocate(block, size);
        if (ptr != nullptr)
        {
            small_allocations.push_back(ptr);
            small_sizes.push_back(size);
            memset(ptr, 0xDD, std::min(size, (std::size_t)256));
        }
    }

    EXPECT_GT(small_allocations.size(), 10000);

    srand(54321);
    int deallocated = 0;
    for (int i = 0; i < 30000; i++)
    {
        if (!small_allocations.empty())
        {
            std::size_t idx = rand() % small_allocations.size();
            block.deallocate(small_allocations[idx], small_sizes[idx]);
            small_allocations.erase(small_allocations.begin() + idx);
            small_sizes.erase(small_sizes.begin() + idx);
            deallocated++;
        }
    }

    EXPECT_GE(deallocated, 20000);

    std::vector<std::size_t> test_sizes = {
        1 * 1024 * 1024,  // 1MB
        5 * 1024 * 1024,  // 5MB
        10 * 1024 * 1024, // 10MB
        20 * 1024 * 1024, // 20MB
        50 * 1024 * 1024, // 50MB
        2 * 1024 * 1024,  // 2MB
        8 * 1024 * 1024,  // 8MB
        15 * 1024 * 1024, // 15MB
        30 * 1024 * 1024, // 30MB
    };

    std::vector<void *> final_allocations;
    int successful_allocs = 0;
    for (std::size_t size : test_sizes)
    {
        void *ptr = allocate(block, size);
        if (ptr != nullptr)
        {
            successful_allocs++;
            final_allocations.push_back(ptr);
            memset(ptr, 0xEE, 4096);
        }
    }

    EXPECT_GT(successful_allocs, 2);

    for (void *ptr : small_allocations)
    {
        block.deallocate(ptr, 1024);
    }

    for (std::size_t i = 0; i < final_allocations.size(); i++)
    {
        block.deallocate(final_allocations[i], test_sizes[i]);
    }
}