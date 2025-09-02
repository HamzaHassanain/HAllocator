#include <iostream>
#include "includes/hh_alloc.hpp"

void print_arr(int *arr, int size)
{
    if (!arr)
        return;

    for (int i = 0; i < size; i++)
    {
        std::cout << arr[i] << " ";
    }
    std::cout << std::endl;
}

struct struct_shit
{
    int x;
    long long y;
    char z;
};

int main()
{
    using namespace hh_alloc;

    std::cout << "Starting allocator stress test...\n\n";

    // Test 1: Basic allocation and deallocation
    std::cout << "Test 1: Basic allocation/deallocation\n";
    void *ptr1 = try_alloc(100);
    void *ptr2 = try_alloc(200);
    void *ptr3 = try_alloc(50);

    std::cout << "Allocated 3 blocks: " << ptr1 << ", " << ptr2 << ", " << ptr3 << std::endl;
    alloc_print();

    hh_alloc::free(ptr2); // Free middle block
    std::cout << "After freeing middle block:\n";
    alloc_print();

    // Test 2: Fragmentation and coalescing
    std::cout << "\nTest 2: Fragmentation and coalescing\n";
    hh_alloc::free(ptr1); // Should merge with freed ptr2
    std::cout << "After freeing first block (should merge):\n";
    alloc_print();

    hh_alloc::free(ptr3); // Should merge with previous merged block
    std::cout << "After freeing last block (should merge all):\n";
    alloc_print();

    // Test 3: Reuse of freed memory
    std::cout << "\nTest 3: Memory reuse\n";
    void *ptr4 = try_alloc(150); // Should reuse freed space
    std::cout << "Allocated 150 bytes (should reuse freed space): " << ptr4 << std::endl;
    alloc_print();

    // Test 4: Multiple small allocations
    std::cout << "\nTest 4: Multiple small allocations\n";
    void *small_ptrs[10];
    for (int i = 0; i < 10; i++)
    {
        // std::cout << "Before Small Alloc " << i << ":\n";
        // alloc_print();
        small_ptrs[i] = try_alloc(32);
        std::cout << "Small alloc " << 32 << ": " << small_ptrs[i] << std::endl;
        // alloc_print();
    }
    alloc_print();

    // Test 5: Free every other block to create fragmentation
    std::cout << "\nTest 5: Creating fragmentation\n";
    for (int i = 1; i < 10; i += 2)
    {
        hh_alloc::free(small_ptrs[i]);
        std::cout << "Freed small block " << i << std::endl;
    }
    alloc_print();

    // Test 6: Try to allocate in fragmented space
    std::cout << "\nTest 6: Allocating in fragmented space\n";
    void *frag_ptr = try_alloc(20); // Should fit in freed fragments
    std::cout << "Allocated 20 bytes in fragmented space: " << frag_ptr << std::endl;
    alloc_print();

    // Test 7: Large allocation
    std::cout << "\nTest 7: Large allocation\n";
    void *large_ptr = try_alloc(4096);
    std::cout << "Allocated large block (4096 bytes): " << large_ptr << std::endl;
    alloc_print();

    // Test 8: Edge cases - zero size and null pointer
    std::cout << "\nTest 8: Edge cases\n";
    void *zero_ptr = try_alloc(0);
    std::cout << "Zero size allocation: " << zero_ptr << std::endl;

    void *null_free = hh_alloc::free(nullptr);
    std::cout << "Free null pointer result: " << null_free << std::endl;

    // Test 9: Stress test with random allocations/deallocations
    std::cout << "\nTest 9: Random stress test\n";
    void *stress_ptrs[20];
    for (int i = 0; i < 20; i++)
    {
        stress_ptrs[i] = nullptr;
    }

    // Random allocation pattern
    for (int round = 0; round < 5; round++)
    {
        std::cout << "Stress round " << round + 1 << std::endl;

        // Allocate some blocks
        for (int i = 0; i < 20; i += 3)
        {
            if (!stress_ptrs[i])
            {
                stress_ptrs[i] = try_alloc((i + 1) * 16);
                std::cout << "  Allocated " << (i + 1) * 16 << " bytes at index " << i << std::endl;
            }
        }

        // Free some blocks
        for (int i = 1; i < 20; i += 4)
        {
            if (stress_ptrs[i])
            {
                hh_alloc::free(stress_ptrs[i]);
                stress_ptrs[i] = nullptr;
                std::cout << "  Freed block at index " << i << std::endl;
            }
        }

        alloc_print();
    }

    // Test 10: Clean up all remaining allocations
    std::cout << "\nTest 10: Final cleanup\n";
    hh_alloc::free(ptr4);
    hh_alloc::free(frag_ptr);
    hh_alloc::free(large_ptr);

    // Free remaining small blocks
    for (int i = 0; i < 10; i += 2)
    {
        if (small_ptrs[i])
        {
            hh_alloc::free(small_ptrs[i]);
        }
    }

    // Free remaining stress test blocks
    for (int i = 0; i < 20; i++)
    {
        if (stress_ptrs[i])
        {
            hh_alloc::free(stress_ptrs[i]);
        }
    }

    std::cout << "After final cleanup:\n";
    alloc_print();

    // Test 11: Boundary conditions
    std::cout << "\nTest 11: Boundary conditions\n";
    int *tiny = (int *)try_alloc(10 * 4); // Minimum allocation
    void *huge = try_alloc(65536);        // Large allocation
    std::cout << "Tiny allocation (40 byte): " << tiny << std::endl;
    std::cout << "Huge allocation (64KB): " << huge << std::endl;
    alloc_print();

    for (int i = 0; i < 10; i++)
    {
        tiny[i] = i;
    }

    try_realloc(tiny, 20 * 4);

    for (int i = 0; i < 20; i++)
    {
        std::cout << "Tiny[" << i << "] = " << tiny[i] << std::endl;
    }
    // hh_alloc::free(tiny);
    // hh_alloc::free(huge);
    int arr[50];
    mem_set(arr, -1, sizeof(arr));
    for (int i = 0; i < 50; i++)
    {
        std::cout << "Arr[" << i << "] = " << arr[i] << std::endl;
    }
    alloc_print();

    std::cout << "\nStress test completed successfully!\n";

    return 0;
}
