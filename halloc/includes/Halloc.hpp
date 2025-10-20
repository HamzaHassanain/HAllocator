/**
 * @file Halloc.hpp
 * @brief High-level allocator interface using Red-Black tree for efficient memory management.
 *
 * This file defines the main Halloc allocator class, which provides a type-safe,
 * scalable memory allocator built on BlocksContainer. It uses Red-Black trees for
 * O(log n) best-fit allocation and supports automatic block creation.
 *
 * Default configuration:
 * - Block size: 256 MB
 * - Maximum blocks: 4 (total capacity: 1 GB)
 */

#pragma once

#include "BlocksContainer.hpp"

const int DEFAULT_BLOCK_SIZE = (256 * 1024 * 1024); ///< Default block size: 256 MB
const int DEFAULT_MAX_NUM_BLOCKS = 4;               ///< Default max blocks: 4

namespace hh::halloc
{
    /**
     * @brief Type-safe allocator using Red-Black tree for best-fit allocation.
     *
     * This class provides a high-level allocator interface compatible with STL containers.
     * It manages multiple memory blocks (via BlocksContainer) and provides type-safe
     * allocation/deallocation for objects of type T.
     *
     * Key features:
     * - Best-fit allocation strategy (minimizes fragmentation)
     * - O(log n) allocation and deallocation (Red-Black tree)
     * - Automatic block coalescing (merges adjacent free blocks)
     * - Automatic block creation up to MaxNumBlocks limit
     * - Thread-unsafe (caller must synchronize)
     *
     * @tparam T Type of objects to allocate (default: void for raw bytes)
     * @tparam BlockSize Size of each memory block in bytes (default: 256 MB)
     * @tparam MaxNumBlocks Maximum number of blocks (default: 4)
     *
     * @note This allocator is NOT thread-safe
     * @note Compatible with STL containers via std::allocator_traits
     */
    template <typename T = void, int BlockSize = DEFAULT_BLOCK_SIZE, int MaxNumBlocks = DEFAULT_MAX_NUM_BLOCKS>
    class Halloc
    {
        BlocksContainer<BlockSize, MaxNumBlocks> blocks; ///< Underlying multi-block container

    public:
        /**
         * @brief Default constructor - creates allocator with one initial block.
         * @post blocks contains one initialized block of size BlockSize
         */
        Halloc();

        /**
         * @brief Allocates memory for 'count' objects of type T.
         *
         * Requests count * sizeof(T) bytes from the underlying BlocksContainer.
         * If no suitable block exists and space is available, creates a new block.
         *
         * @param count Number of objects to allocate space for
         * @return Pointer to allocated memory, or nullptr if allocation fails
         *
         * @pre count > 0
         * @post If successful, returned pointer is valid and aligned for T
         * @post If unsuccessful (nullptr), no state was modified
         *
         * @note Allocation fails if:
         *       - count * sizeof(T) > BlockSize
         *       - All blocks full and MaxNumBlocks reached
         * @note Does NOT construct objects (use placement new if needed)
         */
        T *allocate(std::size_t count);

        /**
         * @brief Deallocates memory previously allocated for 'count' objects.
         *
         * Returns memory to the owning block, marks it as free, and attempts to
         * merge with adjacent free blocks to reduce fragmentation.
         *
         * @param ptr Pointer previously returned by allocate()
         * @param count Number of objects (must match allocate() call)
         *
         * @pre ptr != nullptr
         * @pre ptr was returned by this allocator's allocate()
         * @pre count matches the value passed to allocate()
         * @post Memory is freed and merged with adjacent free blocks
         *
         * @note Destroy objects (call destructors manually if needed)
         * @warning Undefined behavior if ptr was not allocated by this allocator
         */
        void deallocate(T *ptr, std::size_t count);

        /**
         * @brief Equality comparison - checks if allocators are the same object.
         *
         * @param other Another Halloc instance
         * @return true if this == &other, false otherwise
         *
         * @note Only the same allocator instance can deallocate its allocations
         */
        bool operator==(const Halloc &other) const { return this == &other; }

        /**
         * @brief Inequality comparison.
         *
         * @param other Another Halloc instance
         * @return true if this != &other, false otherwise
         */
        bool operator!=(const Halloc &other) const
        {
            return !(*this == other);
        }

        /**
         * @brief Destructor - releases all blocks back to the OS.
         *
         * Each Block destructor will call munmap to return memory to the operating system.
         *
         * @post All memory blocks are returned to OS
         * @warning Do not use any pointers allocated by this allocator after destruction
         */
        ~Halloc();
    };
}
#include "../src/Halloc.ipp"