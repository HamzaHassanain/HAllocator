#pragma once

#include "BlocksContainer.hpp"

const int DEFAULT_BLOCK_SIZE = (256 * 1024 * 1024);
const int DEFAULT_MAX_NUM_BLOCKS = 4;

namespace hh::halloc
{

    template <typename T = void, int BlockSize = DEFAULT_BLOCK_SIZE, int MaxNumBlocks = DEFAULT_MAX_NUM_BLOCKS>
    class Halloc
    {

        BlocksContainer<BlockSize, MaxNumBlocks> blocks;

    public:
        Halloc();

        T *allocate(std::size_t count);
        void deallocate(T *ptr, std::size_t count);

        bool operator==(const Halloc &other) const { return this == &other; }
        bool operator!=(const Halloc &other) const
        {
            return !(*this == other);
        }

        ~Halloc();
    };
}
#include "../src/Halloc.ipp"