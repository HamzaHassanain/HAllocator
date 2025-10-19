/*
Basic memory allocator header file

It is NOT THREAD SAFE.

IT IS VERY SLOW AND INEFFICIENT - FOR EDUCATIONAL PURPOSES ONLY.

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

    /// @brief Size of each memory block
    constexpr mem_size_t BLOCK_SIZE = 4096;

    /// @brief Structure representing the metadata of each memory block
    ///        always lives just before the actual allocated memory
    struct mem_node
    {
        mem_node *nxt, *prv;
        mem_size_t size;
    };

    /// @brief Size of the mem_node structure
    constexpr mem_size_t mem_node_size = sizeof(mem_node);

    /// @brief Pointer to the head of the memory block linked list
    extern mem_node *__head, *__tail;

    /// @brief Check if a block is free
    bool is_free(mem_size_t &size);

    /// @brief Mark a block as free
    void make_free(mem_size_t &size);

    /// @brief Mark a block as used
    void make_used(mem_size_t &size);

    /// @brief Get the size of a block (without the free bit)
    mem_size_t get_size(mem_size_t &size);

    /// @brief Add two memory sizes
    mem_size_t add(mem_size_t a, mem_size_t b);

    /// @brief Subtract two memory sizes
    mem_size_t sub(mem_size_t a, mem_size_t b);

    /// @brief Request form the OS to move the program break by size bytes
    /// @param size  The number of bytes to allocate
    /// @return A pointer to the allocated memory, or nullptr on failure
    void *sbrk_then_alloc(mem_size_t size);

    /// @brief Try to merge the given free node with adjacent free nodes
    /// @param nd  The node to attempt merging on
    void try_merge(mem_node *nd);

    /// @brief Free the given memory block
    /// @param ptr  Pointer to the memory block to free
    /// @return Always returns nullptr
    void *free(void *ptr);

    /// @brief Shrink the given node to the specified size, creating a new free node if necessary
    /// @param nd    The node to shrink
    /// @param size  The new size for the node
    void shrink_then_align(mem_node *nd, mem_size_t size);

    /// @brief Try to allocate a memory block of the given size
    /// @param size  The size of the memory block to allocate
    /// @return A pointer to the allocated memory, or nullptr on failure
    void *try_alloc(mem_size_t size);

    /// @brief  Copy memory from source to destination
    /// @param dest Destination pointer
    /// @param src  Source pointer
    /// @param n  Number of bytes to copy
    void mem_copy(void *dest, const void *src, size_t n);

    /// @brief Try to reallocate a memory block to a new size
    /// @param ptr  Pointer to the memory block to reallocate
    /// @param size  The new size for the memory block
    /// @return A pointer to the reallocated memory, or nullptr on failure
    void *try_realloc(void *ptr, mem_size_t size);

    /// @brief Set a block of memory to a specific value
    /// @param ptr   Pointer to the memory block
    /// @param value Value to set
    /// @param n  Number of bytes to set
    void mem_set(void *ptr, int value, size_t num);

    /// @brief Try to allocate and zero-initialize an array
    /// @param num  Number of elements
    /// @param size  Size of each element
    /// @return A pointer to the allocated memory, or nullptr on failure
    void *try_calloc(size_t num, size_t size);

    /// @brief Print the current allocation status
    /// @return void
    void alloc_print();
};