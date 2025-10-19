#pragma once

#include "Block.hpp"

const int DEFAULT_BLOCK_SIZE = (256 * 1024 * 1024);
const int DEFAULT_MAX_NUM_BLOCKS = 4;

template <typename T = void, int BlockSize = DEFAULT_BLOCK_SIZE, int MaxNumBlocks = DEFAULT_MAX_NUM_BLOCKS>
class halloc
{

    Block *blocks[MaxNumBlocks];

public:
    halloc();

    T *allocate(std::size_t bytes);
    void deallocate(T *ptr, std::size_t bytes);

    template <typename U, typename... Args>
    void construct(Args &&...args);

    template <typename U>
    void destroy(U *ptr);

    bool operator==(const halloc &other) const { return this == &other; }
    bool operator!=(const halloc &other) const
    {
        return !(*this == other);
    }

    ~halloc();
};

#include "../src/halloc.ipp"