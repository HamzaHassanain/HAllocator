#include "../includes/Halloc.hpp"
namespace hh::halloc
{
    template <typename T, int BlockSize, int MaxNumBlocks>
    Halloc<T, BlockSize, MaxNumBlocks>::Halloc()
    {
        for (int i = 0; i < MaxNumBlocks; i++)
        {
            blocks[i] = nullptr;
        }

        current_block_index = 0;
        blocks[current_block_index] = new Block(BlockSize);
    }

    template <typename T, int BlockSize, int MaxNumBlocks>
    T *Halloc<T, BlockSize, MaxNumBlocks>::allocate(std::size_t count)
    {
        return static_cast<T *>(blocks.allocate(count * sizeof(T)));
    }

    template <typename T, int BlockSize, int MaxNumBlocks>
    void Halloc<T, BlockSize, MaxNumBlocks>::deallocate(T *ptr, std::size_t count)
    {
        blocks.deallocate(ptr, count * sizeof(T));
    }

    template <typename T, int BlockSize, int MaxNumBlocks>
    Halloc<T, BlockSize, MaxNumBlocks>::~Halloc()
    {
    }
}