/**
 * @file BlocksContainer.hpp
 * @brief Container managing multiple memory blocks for large-scale allocations.
 *
 * This file defines a template class that manages multiple Block instances to support
 * allocations beyond a single block's capacity. It provides automatic block creation
 * and best-fit search across all blocks.
 */

#pragma once

#include <cstddef>
#include <limits>
#include <utility>

#include "Block.hpp"

namespace hh::halloc {
/**
 * @brief Manages multiple memory blocks for scalable allocation.
 *
 * This template class provides a higher-level allocator that can create and manage
 * multiple Block instances. When an allocation cannot be satisfied by existing blocks,
 * a new block is created automatically (up to MaxNumBlocks limit).
 *
 * The container performs best-fit search across ALL blocks to minimize fragmentation.
 *
 * @tparam BlockSize Size of each memory block in bytes
 * @tparam MaxNumBlocks Maximum number of blocks allowed
 *
 * @note Thread-safety: This class is NOT thread-safe
 * @note Memory overhead: Maintains an array of Block objects
 */
template <std::size_t BlockSize, int MaxNumBlocks>
class BlocksContainer {
    Block blocks[MaxNumBlocks];  ///< Array of memory blocks
    int current_block_index;     ///< Index of the last created block (-1 if none)

    /**
     * @brief Finds the best-fit free node across all initialized blocks.
     *
     * Searches all created blocks for the smallest free node that can satisfy
     * the requested size. This minimizes fragmentation compared to first-fit.
     *
     * @param bytes Requested allocation size (excluding metadata)
     * @return Pair of (block_index, node_pointer)
     *         - block_index: Index in blocks array (or MaxNumBlocks if not found)
     *         - node_pointer: Pointer to best-fit MemoryNode (or nullptr if not found)
     *
     * @pre bytes > 0
     * @post If found, 0 <= block_index < current_block_index
     * @post If found, node != nullptr and get_actual_value(node->value) >= bytes
     *
     * @note Time complexity: O(MaxNumBlocks * log(nodes_per_block))
     */
    std::pair<std::size_t, MemoryNode*> best_fit(std::size_t bytes);

public:
    /**
     * @brief Default constructor - initializes container with no blocks.
     * @post current_block_index == -1 (no blocks created yet)
     */
    BlocksContainer();

    /**
     * @brief Allocates memory from the container.
     *
     * Algorithm:
     * 1. Search all existing blocks for best-fit node
     * 2. If found, allocate from that block
     * 3. If not found and space available, create new block and allocate
     * 4. If no space for new block, return nullptr (allocation failure)
     *
     * @param bytes Number of bytes to allocate
     * @return Pointer to allocated memory, or nullptr if allocation fails
     *
     * @pre bytes > 0
     * @post If successful, returned pointer is valid and points to usable memory
     * @post If unsuccessful (nullptr), no blocks were modified
     *
     * @note Fails if bytes > BlockSize or current_block_index >= MaxNumBlocks
     */
    void* allocate(std::size_t bytes);

    /**
     * @brief Deallocates previously allocated memory.
     *
     * Determines which block owns the memory pointer and delegates deallocation
     * to that block. The block will merge adjacent free nodes automatically.
     *
     * @param ptr Pointer previously returned by allocate()
     * @param bytes Size of the allocation (used to determine owning block)
     *
     * @pre ptr != nullptr
     * @pre ptr was returned by this container's allocate()
     * @pre bytes matches the size passed to allocate()
     * @post Memory is marked as free and merged with adjacent free blocks
     *
     * @warning Undefined behavior if ptr was not allocated by this container
     */
    void deallocate(void* ptr, std::size_t bytes);
};
}  // namespace hh::halloc

#include "../src/BlocksContainer.ipp"