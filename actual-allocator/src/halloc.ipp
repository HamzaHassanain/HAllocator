#include "../includes/halloc.hpp"

template <typename T, int BlockSize, int MaxNumBlocks>
halloc<T, BlockSize, MaxNumBlocks>::halloc()
{
    for (int i = 0; i < MaxNumBlocks; i++)
    {
        blocks[i] = nullptr;
    }
}