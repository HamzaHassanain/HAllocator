/**
 * @file basic_alloc.hpp
 * @brief Basic memory allocator interface - educational implementation.
 *
 * This file provides a simple linked-list-based memory allocator using sbrk() to request
 * memory from the operating system. It uses first-fit allocation strategy and maintains
 * a singly-linked list of free/used blocks.
 *
 * @warning NOT THREAD SAFE - do not use in multi-threaded environments
 * @warning VERY SLOW AND INEFFICIENT - for educational purposes only
 * @warning Production code should use halloc or standard allocators
 *
 * Implementation details:
 * - Uses program break (sbrk) for memory acquisition
 * - First-fit allocation strategy (O(n) search)
 * - Automatic coalescing of adjacent free blocks
 * - Minimum fragment size to prevent excessive splitting
 * - Metadata stored before each allocated block
 */

#pragma once
#include <iostream>
#include <unistd.h>
#include <climits>
#include <stdexcept>
#include <cstring>
#include <cstdio>

namespace hh::basic_alloc
{
    /// @brief Type definition for memory size
    using mem_size_t = unsigned long long;

    /// @brief Minimum fragment size to consider splitting a block
    constexpr mem_size_t MIN_FRAGMENT_SIZE = 32;

    /// @brief Size of each memory block requested from OS via sbrk
    constexpr mem_size_t BLOCK_SIZE = 4096;

    /**
     * @brief Metadata structure for each memory block.
     *
     * This structure lives immediately before the user-visible memory.
     * It forms a doubly-linked list of all blocks (free and used).
     *
     * Memory layout:
     * [mem_node metadata] [user memory ...]
     *                     ^- pointer returned to user
     *
     * The size field encodes both size and free/used status:
     * - Bit 0: free (1) or used (0)
     * - Bits 1-63: actual size in bytes
     */
    struct mem_node
    {
        mem_node *nxt;   ///< Pointer to next block in list
        mem_node *prv;   ///< Pointer to previous block in list
        mem_size_t size; ///< Size in bytes (bit 0 = free flag)
    };

    /// @brief Size of the mem_node structure
    constexpr mem_size_t mem_node_size = sizeof(mem_node);

    /// @brief Pointer to the head of the memory block linked list
    extern mem_node *__head, *__tail;

    /**
     * @brief Check if a block is free.
     * @param size Size field from mem_node
     * @return true if block is free (bit 0 == 1), false if used
     */
    bool is_free(mem_size_t &size);

    /**
     * @brief Mark a block as free.
     * @param size Size field from mem_node
     * @post Bit 0 of size is set to 1
     */
    void make_free(mem_size_t &size);

    /**
     * @brief Mark a block as used.
     * @param size Size field from mem_node
     * @post Bit 0 of size is cleared to 0
     */
    void make_used(mem_size_t &size);

    /**
     * @brief Get the actual size of a block (excluding free bit).
     * @param size Size field from mem_node
     * @return Size in bytes (bit 0 masked out)
     */
    mem_size_t get_size(mem_size_t &size);

    /**
     * @brief Add two memory sizes with overflow checking.
     * @param a First operand
     * @param b Second operand
     * @return a + b
     * @throw std::overflow_error if addition overflows
     */
    mem_size_t add(mem_size_t a, mem_size_t b);

    /**
     * @brief Subtract two memory sizes with underflow checking.
     * @param a Minuend
     * @param b Subtrahend
     * @return a - b
     * @throw std::underflow_error if subtraction underflows
     */
    mem_size_t sub(mem_size_t a, mem_size_t b);

    /**
     * @brief Request memory from OS and allocate.
     *
     * Uses sbrk() to extend the program break by at least size bytes
     * (rounded up to BLOCK_SIZE multiples). Creates a new mem_node and
     * returns pointer to usable memory.
     *
     * @param size Number of bytes requested
     * @return Pointer to allocated memory, or nullptr on failure
     *
     * @post Program break is extended
     * @post New block is added to linked list
     * @post Block is marked as used
     */
    void *sbrk_then_alloc(mem_size_t size);

    /**
     * @brief Merge adjacent free blocks to reduce fragmentation.
     *
     * Attempts to merge the given free node with:
     * 1. Next node (if free)
     * 2. Previous node (if free)
     *
     * @param nd Node to merge (must be free)
     * @pre nd != nullptr
     * @pre is_free(nd->size) == true
     * @post Adjacent free blocks are coalesced
     */
    void try_merge(mem_node *nd);

    /**
     * @brief Free a previously allocated memory block.
     *
     * Marks the block as free and attempts to merge with adjacent free blocks.
     *
     * @param ptr Pointer to memory (returned by try_alloc)
     * @return Always returns nullptr
     *
     * @pre ptr != nullptr
     * @pre ptr was allocated by try_alloc
     * @post Block is marked as free
     * @post Block is merged with adjacent free blocks if possible
     */
    void *free(void *ptr);

    /**
     * @brief Shrink a block and create a new free block from remainder.
     *
     * If the node is significantly larger than requested size, splits it:
     * - Current node becomes 'size' bytes (used)
     * - Remainder becomes a new free node
     *
     * @param nd Node to shrink
     * @param size Desired size for node
     *
     * @pre nd != nullptr
     * @post nd->size == size (if split occurred)
     * @post If split, a new free node exists after nd
     */
    void shrink_then_align(mem_node *nd, mem_size_t size);

    /**
     * @brief Allocate a memory block using first-fit strategy.
     *
     * Searches the linked list for the first free block large enough
     * to satisfy the request. If found, splits if necessary and marks as used.
     * If not found, requests more memory from OS via sbrk_then_alloc.
     *
     * @param size Number of bytes to allocate
     * @return Pointer to allocated memory, or nullptr on failure
     *
     * @post If successful, returned pointer is valid
     * @post Block is marked as used
     *
     * @note Time complexity: O(n) where n = number of blocks
     */
    void *try_alloc(mem_size_t size);

    /**
     * @brief Copy memory from source to destination.
     *
     * @param dest Destination pointer
     * @param src Source pointer
     * @param n Number of bytes to copy
     *
     * @pre dest and src do not overlap
     * @post n bytes copied from src to dest
     */
    void mem_copy(void *dest, const void *src, size_t n);

    /**
     * @brief Reallocate a memory block to a new size.
     *
     * Attempts to resize the block in place if possible. Otherwise:
     * 1. Allocate new block of requested size
     * 2. Copy old data to new block
     * 3. Free old block
     *
     * @param ptr Pointer to existing allocation
     * @param size New size in bytes
     * @return Pointer to resized memory, or nullptr on failure
     *
     * @pre ptr was allocated by try_alloc (or nullptr)
     * @post If successful, old data is preserved (up to min(old_size, new_size))
     * @post Old pointer may be invalidated
     */
    void *try_realloc(void *ptr, mem_size_t size);

    /**
     * @brief Set a block of memory to a specific value.
     *
     * @param ptr Pointer to memory block
     * @param value Value to set (cast to unsigned char)
     * @param num Number of bytes to set
     *
     * @post num bytes at ptr are set to value
     */
    void mem_set(void *ptr, int value, size_t num);

    /**
     * @brief Allocate and zero-initialize an array.
     *
     * Allocates num * size bytes and initializes all bytes to zero.
     *
     * @param num Number of elements
     * @param size Size of each element
     * @return Pointer to zero-initialized memory, or nullptr on failure
     *
     * @post All bytes in allocated memory are zero
     */
    void *try_calloc(size_t num, size_t size);

    /**
     * @brief Print allocation status for debugging.
     *
     * Displays all blocks (free and used) with their sizes and addresses.
     *
     * @post Allocator state is unchanged
     * @note For debugging purposes only
     */
    void alloc_print();
};