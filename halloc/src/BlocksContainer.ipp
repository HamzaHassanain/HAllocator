/**
 * @file BlocksContainer.ipp
 * @brief Implementation of BlocksContainer template class methods.
 *
 * This file contains the implementation of multi-block memory management,
 * including best-fit search across blocks and automatic block creation.
 */

#include "../includes/BlocksContainer.hpp"

namespace hh::halloc
{
    /**
     * @brief Helper function to extract actual size from encoded value.
     *
     * Removes the color and status bits (bits 62-63) from the encoded value
     * to get the actual size in bytes.
     *
     * @param value Encoded value with color and status bits
     * @return Actual size in bytes (bits 0-61)
     */
    inline std::size_t get_actual_value(std::size_t value)
    {
        return value & ~(3ull << 62);
    }

    /**
     * @brief Finds the best-fit free node across all initialized blocks.
     *
     * Iterates through all created blocks (0 to current_block_index) and finds
     * the smallest free node that can satisfy the requested size.
     *
     * Algorithm:
     * 1. Initialize best_size to maximum (worst fit)
     * 2. For each block:
     *    a. Call block's best_fit()
     *    b. If found and smaller than current best, update best node
     * 3. Return (block_index, node_pointer)
     *
     * @tparam BlockSize Size of each memory block
     * @tparam MaxNumBlocks Maximum number of blocks
     * @param bytes Requested allocation size
     * @return Pair (block_index, node_pointer)
     *         - If found: block_index in [0, current_block_index], node != nullptr
     *         - If not found: block_index == max(size_t), node == nullptr
     */
    template <std::size_t BlockSize, int MaxNumBlocks>
    std::pair<std::size_t, MemoryNode *> BlocksContainer<BlockSize, MaxNumBlocks>::best_fit(std::size_t bytes)
    {
        std::size_t best_block_index = std::numeric_limits<std::size_t>::max();
        std::size_t best_size = std::numeric_limits<std::size_t>::max();
        MemoryNode *best_node = nullptr;

        // Search all initialized blocks
        for (int i = 0; i <= current_block_index; i++)
        {
            MemoryNode *node = blocks[i].best_fit(bytes);
            if (node)
            {
                std::size_t node_size = get_actual_value(node->value);
                // Track smallest fit
                if (node_size < best_size)
                {
                    best_node = node;
                    best_size = node_size;
                    best_block_index = i;
                }
            }
        }

        return {best_block_index, best_node};
    }

    /**
     * @brief Constructor - creates container with one initial block.
     *
     * Initializes the container by creating the first block of size BlockSize.
     * Additional blocks are created on-demand during allocation.
     *
     * @tparam BlockSize Size of each memory block
     * @tparam MaxNumBlocks Maximum number of blocks
     * @post current_block_index == 0
     * @post blocks[0] is initialized with BlockSize bytes
     */
    template <std::size_t BlockSize, int MaxNumBlocks>
    BlocksContainer<BlockSize, MaxNumBlocks>::BlocksContainer()
    {
        current_block_index = 0;
        blocks[current_block_index] = std::move(Block(BlockSize));
    }

    /**
     * @brief Allocates memory from the container.
     *
     * Algorithm:
     * 1. Search all blocks for best-fit node
     * 2. If found:
     *    a. Allocate from that block
     * 3. If not found:
     *    a. Check if we can create a new block (current_block_index + 1 < MaxNumBlocks)
     *    b. If yes: create new block, find best-fit in new block, allocate
     *    c. If no: return nullptr (allocation failure)
     *
     * @tparam BlockSize Size of each memory block
     * @tparam MaxNumBlocks Maximum number of blocks
     * @param bytes Number of bytes to allocate
     * @return Pointer to allocated memory, or nullptr if allocation fails
     *
     * @note Allocation fails if:
     *       - bytes > BlockSize (too large for any block)
     *       - All blocks full and current_block_index + 1 >= MaxNumBlocks
     */
    template <std::size_t BlockSize, int MaxNumBlocks>
    void *BlocksContainer<BlockSize, MaxNumBlocks>::allocate(std::size_t bytes)
    {
        if (bytes < 1)
        {
            throw std::invalid_argument("Bytes must be positive");
        }

        auto [index, node] = best_fit(bytes);

        // No suitable node found in existing blocks
        if (index == std::numeric_limits<std::size_t>::max())
        {
            // Try to create a new block
            if (current_block_index + 1 < MaxNumBlocks)
            {
                current_block_index++;
                blocks[current_block_index] = std::move(Block(BlockSize));
                index = current_block_index;
                node = blocks[index].best_fit(bytes);
            }
            else
            {
                // No space for new block
                return nullptr;
            }
        }

        // Guard Allocation Against Not Enough Memory
        if (!node)
        {
            return nullptr;
        }

        // Allocate from the selected block
        return blocks[index].allocate(bytes, node);
    }

    /**
     * @brief Deallocates memory by finding the owning block.
     *
     * Searches through blocks to find which one owns the given pointer,
     * then delegates deallocation to that block.
     *
     * Algorithm:
     * 1. For each block i in [0, current_block_index]:
     *    a. Check if blocks[i].head <= ptr < blocks[i+1].head
     *    b. If yes, call blocks[i].deallocate()
     *
     * @tparam BlockSize Size of each memory block
     * @tparam MaxNumBlocks Maximum number of blocks
     * @param ptr Pointer to deallocate
     * @param bytes Size of allocation (for block compatibility)
     *
     * @pre ptr was allocated by this container
     * @post Memory is freed and merged with adjacent free blocks
     *
     * @warning Undefined behavior if ptr was not allocated by this container
     */
    template <std::size_t BlockSize, int MaxNumBlocks>
    void BlocksContainer<BlockSize, MaxNumBlocks>::deallocate(void *ptr, std::size_t bytes)
    {
        // Find which block owns this pointer
        for (int i = 0; i <= current_block_index; i++)
        {
            // Check if ptr is within block i's address range
            void *block_start = blocks[i].get_head();
            void *block_end = (char *)block_start + BlockSize;

            if (block_start <= ptr && ptr < block_end)
            {
                blocks[i].deallocate(ptr, bytes);
                return;
            }
        }

        throw std::invalid_argument("Pointer not allocated by this container");
    }
};