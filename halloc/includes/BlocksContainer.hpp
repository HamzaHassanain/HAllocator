#pragma once

#include <cstddef>
#include <limits>
#include "Block.hpp"
namespace hh::halloc
{

    template <std::size_t BlockSize, int MaxNumBlocks>
    class BlocksContainer
    {
        Block blocks[MaxNumBlocks];
        int current_block_index;

        std::size_t best_fit(std::size_t bytes)
        {
            std::size_t best_index = std::numeric_limits<std::size_t>::max();
            std::size_t best_size = std::numeric_limits<std::size_t>::max();
            for (int i = 0; i <= current_block_index; i++)
            {
                MemoryNode *node = blocks[i].best_fit(bytes);
                if (node)
                {
                    std::size_t node_size = get_actual_value(node->value);
                    if (node_size < best_size)
                    {
                        best_size = node_size;
                        best_index = i;
                    }
                }
            }

            return best_index;
        }

    public:
        BlocksContainer()
        {
            current_block_index = 0;
            blocks[current_block_index] = std::move(Block(BlockSize));
        }

        void *allocate(std::size_t bytes)
        {
            auto index = best_fit(bytes);
            if (index == std::numeric_limits<std::size_t>::max())
            {
                if (current_block_index + 1 < MaxNumBlocks)
                {
                    current_block_index++;
                    blocks[current_block_index] = std::move(Block(BlockSize));
                    index = current_block_index;
                }
                else
                {
                    return nullptr;
                }
            }
            return blocks[index].allocate(bytes);
        }
        void deallocate(void *ptr, std::size_t bytes)
        {
            ptr -= MEMORY_NODE_SIZE;
            auto *node = (MemoryNode *)ptr;
            auto root = node;
            while (root->parent)
            {
                root = root->parent;
            }

            for (int i = 0; i <= current_block_index; i++)
            {
                if (blocks[i].head == root)
                {
                    blocks[i].deallocate(ptr, bytes);
                    return;
                }
            }
        }
    };
}