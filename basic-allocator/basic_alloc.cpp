/**
 * @file basic_alloc.cpp
 * @brief Implementation of basic linked-list memory allocator.
 *
 * This file implements a simple educational memory allocator using:
 * - Doubly-linked list of blocks (free and used)
 * - First-fit allocation strategy
 * - sbrk() for memory acquisition
 * - Automatic coalescing of adjacent free blocks
 * - Bit 63 of size field for free/used flag
 *
 * @warning NOT for production use - educational purposes only
 */

#include "basic_alloc.hpp"

#include <unistd.h>

#include <climits>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <stdexcept>

namespace hh::basic_alloc {
// Global pointers to head and tail of block list
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
MemNode *__head = nullptr, *__tail = nullptr;

/**
 * @brief Check if block is free using bit 63.
 * @param size Size field from MemNode
 * @return true if bit 63 is set (free), false otherwise (used)
 */
inline bool is_free(MemSizeT& size) {
    return (size & (1ULL << 63)) != 0U;
}

/**
 * @brief Mark block as free by setting bit 63.
 * @param size Size field from MemNode
 * @post Bit 63 of size is set to 1
 */
inline void make_free(MemSizeT& size) {
    size |= (1ULL << 63);
}

/**
 * @brief Mark block as used by clearing bit 63.
 * @param size Size field from MemNode
 * @post Bit 63 of size is cleared to 0
 */
inline void make_used(MemSizeT& size) {
    size &= ~(1ULL << 63);
}

/**
 * @brief Extract actual size by masking off bit 63.
 * @param size Size field from MemNode
 * @return Size in bytes without free/used bit
 */
inline MemSizeT get_size(MemSizeT& size) {
    return (size & ~(1ULL << 63));
}

/**
 * @brief Add two sizes (clearing free bits first).
 * @param a First operand
 * @param b Second operand
 * @return a + b with free bits cleared
 */
inline MemSizeT add(MemSizeT a, MemSizeT b) {
    make_used(a);
    make_used(b);
    return a + b;
}

/**
 * @brief Subtract two sizes (clearing free bits first).
 * @param a Minuend
 * @param b Subtrahend
 * @return a - b with free bits cleared
 */
inline MemSizeT sub(MemSizeT a, MemSizeT b) {
    make_used(a);
    make_used(b);
    return a - b;
}

/**
 * @brief Request memory from OS using sbrk and allocate.
 *
 * Extends program break by size + metadata, creates new MemNode,
 * and adds it to the tail of the linked list.
 *
 * @param size Requested allocation size (excluding metadata)
 * @return Pointer to usable memory (after MemNode header)
 * @throw std::bad_alloc if sbrk fails
 */
void* sbrk_then_alloc(MemSizeT size) {
    // Request memory from OS
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast,google-readability-casting)
    auto* nxt_node_addr = static_cast<MemNode*>(sbrk(static_cast<intptr_t>(size + MEM_NODE_SIZE)));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast,performance-no-int-to-ptr)
    if (nxt_node_addr == reinterpret_cast<MemNode*>(-1)) {
        throw std::bad_alloc();
    }

    // Initialize new node
    nxt_node_addr->size = size;
    make_used(nxt_node_addr->size);

    // Add to linked list
    if (__head == nullptr) {
        __head = nxt_node_addr;
        __tail = nxt_node_addr;
    } else {
        __tail->nxt = nxt_node_addr;
        nxt_node_addr->prv = __tail;
        __tail = nxt_node_addr;
    }
    __tail->nxt = nullptr;

    // Return pointer to usable memory (skip metadata)
    return static_cast<void*>(nxt_node_addr + 1);
}

/**
 * @brief Merge a free node with adjacent free nodes.
 *
 * Attempts to coalesce:
 * 1. Forward: if next node is free, merge into current
 * 2. Backward: if previous node is free, merge current into previous
 *
 * This reduces fragmentation by combining adjacent free blocks.
 *
 * @param nd Node to merge (must be free)
 * @pre nd is marked as free
 * @post Adjacent free blocks are merged
 */
void coalesce_nodes(MemNode* nd) {
    if (nd == nullptr) {
        return;
    }

    // Forward merge: merge with next node if it's free
    if (nd->nxt != nullptr && is_free(nd->nxt->size)) {
        if (__tail == nd->nxt) {
            __tail = nd;
        }

        // Combine sizes (include metadata of next node)
        nd->size = add(nd->size, nd->nxt->size);
        nd->size = add(nd->size, MEM_NODE_SIZE);
        make_free(nd->size);

        // Update linked list
        nd->nxt = nd->nxt->nxt;
        if (nd->nxt != nullptr) {
            nd->nxt->prv = nd;
        }
    }

    // Backward merge: merge with previous node if it's free
    if (nd->prv != nullptr && is_free(nd->prv->size)) {
        if (__tail == nd) {
            __tail = nd->prv;
        }

        // Combine sizes (include metadata of current node)
        nd->prv->size = add(nd->prv->size, nd->size);
        nd->prv->size = add(nd->prv->size, MEM_NODE_SIZE);
        make_free(nd->prv->size);

        // Update linked list
        nd->prv->nxt = nd->nxt;
        if (nd->nxt != nullptr) {
            nd->nxt->prv = nd->prv;
        }
    }

    if (__tail != nullptr) {
        __tail->nxt = nullptr;
    }
}

/**
 * @brief Free a memory block and merge with adjacent free blocks.
 *
 * @param ptr Pointer to memory (returned by try_alloc)
 * @return Always nullptr
 *
 * @post Block is marked as free and merged if possible
 */
void* free(void* ptr) {
    if (ptr == nullptr) {
        return nullptr;
    }

    // Get MemNode pointer (immediately before user memory)
    auto* nd = static_cast<MemNode*>(ptr) - 1;
    make_free(nd->size);

    // Attempt to merge with adjacent free blocks
    coalesce_nodes(nd);
    return nullptr;
}

/**
 * @brief Shrink a block and create a new free block from remainder.
 *
 * If the fragment is large enough (> MIN_FRAGMENT_SIZE + metadata),
 * splits the block:
 * - Current node becomes 'size' bytes (used)
 * - Remainder becomes a new free node
 *
 * @param nd Node to shrink
 * @param size Desired size for node
 *
 * @post If split: nd->size == size and new free node created
 * @post If no split: nd remains unchanged
 */
void shrink_then_align(MemNode* nd, MemSizeT size) {
    MemSizeT fragment = sub(nd->size, size);

    // Only split if fragment is large enough
    if (fragment > MIN_FRAGMENT_SIZE + MEM_NODE_SIZE) {
        // Create new node in remainder space
        auto* new_node = reinterpret_cast<MemNode*>(reinterpret_cast<char*>(nd + 1) + size);
        new_node->size = sub(fragment, MEM_NODE_SIZE);
        make_free(new_node->size);

        // Insert new node into linked list after current
        new_node->nxt = nd->nxt;
        new_node->prv = nd;

        if (nd->nxt != nullptr) {
            nd->nxt->prv = new_node;
        }

        nd->size = size;
        make_used(nd->size);

        nd->nxt = new_node;

        // Update tail if necessary
        if (__tail == nd) {
            __tail = new_node;
        } else {
            coalesce_nodes(new_node);  // Merge with next if possible
        }
    }

    if (__tail != nullptr) {
        __tail->nxt = nullptr;
    }
}

/**
 * @brief Allocate memory using first-fit strategy.
 *
 * Searches linked list for first free block large enough.
 * If found, allocates from that block (splitting if necessary).
 * If not found, requests more memory from OS via sbrk.
 *
 * @param size Number of bytes to allocate
 * @return Pointer to allocated memory, or nullptr if size is 0
 *
 * @note Time complexity: O(n) where n = number of blocks
 */
void* try_alloc(MemSizeT size) {
    if (size == 0U) {
        return nullptr;
    }

    // First-fit: search for first suitable free block
    for (auto* it = __head; it != nullptr; it = it->nxt) {
        if (is_free(it->size) && get_size(it->size) >= size) {
            make_used(it->size);
            shrink_then_align(it, size);
            return static_cast<void*>(it + 1);  // Return pointer after metadata
        }
    }

    // No suitable block found, request from OS
    return sbrk_then_alloc(size);
}

/**
 * @brief Copy n bytes from source to destination.
 *
 * @param dest Destination pointer
 * @param src Source pointer
 * @param n Number of bytes to copy
 *
 * @note Simple byte-by-byte copy (not optimized)
 */
void mem_copy(void* dest, const void* src, size_t n) {
    if (dest == nullptr || src == nullptr || n == 0) {
        return;
    }

    auto* d = static_cast<char*>(dest);
    const auto* s = static_cast<const char*>(src);
    for (size_t i = 0; i < n; ++i) {
        d[i] = s[i];
    }
}

/**
 * @brief Reallocate a memory block to a new size.
 *
 * If new size fits in current block, shrinks in place.
 * Otherwise, allocates new block, copies data, and frees old block.
 *
 * @param ptr Pointer to existing allocation (or nullptr)
 * @param size New size in bytes
 * @return Pointer to resized memory
 *
 * @note If ptr is nullptr, behaves like try_alloc
 */
void* try_realloc(void* ptr, MemSizeT size) {
    if (ptr == nullptr) {
        return try_alloc(size);
    }

    auto* nd = static_cast<MemNode*>(ptr) - 1;

    // If current block is large enough, shrink in place
    if (get_size(nd->size) >= size) {
        shrink_then_align(nd, size);
        return ptr;
    }

    // Allocate new block, copy data, free old
    void* new_ptr = try_alloc(size);
    if (new_ptr != nullptr) {
        mem_copy(new_ptr, ptr, get_size(nd->size));
        free(ptr);
    }
    return new_ptr;
}

/**
 * @brief Set num bytes to specified value.
 *
 * @param ptr Pointer to memory
 * @param value Value to set (cast to unsigned char)
 * @param num Number of bytes to set
 */
void mem_set(void* ptr, int value, size_t num) {
    if (ptr == nullptr || num == 0) {
        return;
    }

    auto* p = static_cast<unsigned char*>(ptr);
    for (size_t i = 0; i < num; i++) {
        p[i] = static_cast<unsigned char>(value);
    }
}

/**
 * @brief Allocate and zero-initialize an array.
 *
 * @param num Number of elements
 * @param size Size of each element
 * @return Pointer to zero-initialized memory, or nullptr on failure
 *
 * @note Checks for overflow before allocation
 */
void* try_calloc(size_t num, size_t size) {
    if (num == 0 || size == 0) {
        return nullptr;
    }

    // Check for multiplication overflow
    if (num > ULLONG_MAX / size) {
        return nullptr;
    }

    void* ptr = try_alloc(num * size);
    if (ptr != nullptr) {
        mem_set(ptr, 0, num * size);
    }
    return ptr;
}

/**
 * @brief Print allocation status table for debugging.
 *
 * Displays all blocks with:
 * - Address
 * - Size (excluding metadata)
 * - Total size (including metadata)
 * - Status (FREE/USED)
 * - Previous and next pointers
 * - Summary statistics
 */
void alloc_print() {
    std::cout << "\n+------------------------------------------------------------------------------"
                 "----------------------+\n";
    std::cout << "|                               Memory Allocation Status                         "
                 "                    |\n";
    std::cout << "+-----------------+----------+---------------+--------------+------------------+-"
                 "--------------------+\n";
    std::cout << "|     Address     |   Size   |  Total Size   |    Status    |        Prev      | "
                 "       Next         |\n";
    std::cout << "+-----------------+----------+---------------+--------------+------------------+-"
                 "--------------------+\n";

    int block_count = 0;
    MemSizeT total_allocated = 0;
    MemSizeT total_free = 0;

    for (auto* it = __head; it != nullptr; it = it->nxt) {
        block_count++;
        MemSizeT block_size = get_size(it->size);
        MemSizeT total_size = block_size + MEM_NODE_SIZE;
        bool is_block_free = is_free(it->size);

        if (is_block_free) {
            total_free += block_size;
        } else {
            total_allocated += block_size;
        }

        // Convert addresses to decimal for display
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        auto addr_decimal = reinterpret_cast<unsigned long long>(it);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        auto prev_decimal =
            it->prv != nullptr ? reinterpret_cast<unsigned long long>(it->prv) : 0ULL;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        auto next_decimal =
            it->nxt != nullptr ? reinterpret_cast<unsigned long long>(it->nxt) : 0ULL;

        constexpr unsigned long long NULL_MARKER = 10000000000000ULL;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg,modernize-use-std-print)
        printf("| %15llu | %8llu | %13llu | %12s | %16llu | %19llu |\n", addr_decimal, block_size,
               total_size, is_block_free ? "    FREE    " : "   USED    ",
               prev_decimal != 0ULL ? prev_decimal : NULL_MARKER,
               next_decimal != 0ULL ? next_decimal : NULL_MARKER);
    }

    std::cout << "+-----------------+----------+---------------+--------------+------------------+-"
                 "--------------------+\n";
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg,modernize-use-std-print)
    printf(
        "| Summary: %d blocks | Allocated: %llu bytes | Free: %llu bytes | Total: %llu bytes    "
        "          |\n",
        block_count, total_allocated, total_free, total_allocated + total_free);
    std::cout << "+------------------------------------------------------------------------------"
                 "----------------------+\n";
}

};  // namespace hh::basic_alloc