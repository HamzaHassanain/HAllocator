/**
 * @file Halloc.ipp
 * @brief Implementation of Halloc template class methods.
 *
 * This file contains the implementation of the high-level Halloc allocator,
 * providing type-safe wrappers around BlocksContainer operations.
 */

#include "../includes/Halloc.hpp"

namespace hh::halloc {
/**
 * @brief Constructor - initializes allocator with BlocksContainer.
 *
 * Creates a new BlocksContainer via shared_ptr. The container automatically
 * initializes with one block. Multiple Halloc instances can share the same
 * container via copy construction.
 *
 * @tparam T Type of objects to allocate
 * @tparam BlockSize Size of each block in bytes
 * @tparam MaxNumBlocks Maximum number of blocks
 * @post blocks points to a new BlocksContainer with one block of size BlockSize
 */
template <typename T, int BlockSize, int MaxNumBlocks>
Halloc<T, BlockSize, MaxNumBlocks>::Halloc()
    : blocks(std::make_shared<BlocksContainer<BlockSize, MaxNumBlocks>>()) {
    // BlocksContainer constructor handles initialization
}

/**
 * @brief Allocates memory for 'count' objects of type T.
 *
 * Calculates total size as count * sizeof(T) and requests allocation
 * from the BlocksContainer. The container performs best-fit search
 * across all blocks and creates new blocks if needed.
 *
 * @tparam T Type of objects to allocate
 * @tparam BlockSize Size of each block in bytes
 * @tparam MaxNumBlocks Maximum number of blocks
 * @param count Number of objects to allocate space for
 * @return Typed pointer to allocated memory, or nullptr if allocation fails
 *
 * @note Does NOT call constructors - caller must use placement new
 */
template <typename T, int BlockSize, int MaxNumBlocks>
T* Halloc<T, BlockSize, MaxNumBlocks>::allocate(std::size_t count) {
    return static_cast<T*>(blocks->allocate(count * sizeof(T)));
}

/**
 * @brief Deallocates memory for 'count' objects of type T.
 *
 * Calculates total size as count * sizeof(T) and passes to BlocksContainer
 * for deallocation. The container finds the owning block and performs
 * deallocation with automatic coalescing.
 *
 * @tparam T Type of objects (must match allocate call)
 * @tparam BlockSize Size of each block in bytes
 * @tparam MaxNumBlocks Maximum number of blocks
 * @param ptr Pointer to deallocate
 * @param count Number of objects (must match allocate call)
 *
 * @note Does NOT call destructors - caller must destroy objects manually
 */
template <typename T, int BlockSize, int MaxNumBlocks>
void Halloc<T, BlockSize, MaxNumBlocks>::deallocate(T* ptr, std::size_t count) {
    blocks->deallocate(ptr, count * sizeof(T));
}

/**
 * @brief Destructor - releases resources.
 *
 * When the last Halloc instance sharing a BlocksContainer is destroyed,
 * the shared_ptr automatically destructs the BlocksContainer, which
 * releases all blocks via their destructors (munmap).
 *
 * @tparam T Type of objects
 * @tparam BlockSize Size of each block in bytes
 * @tparam MaxNumBlocks Maximum number of blocks
 * @post If this was the last reference, all memory is returned to the OS
 */
template <typename T, int BlockSize, int MaxNumBlocks>
Halloc<T, BlockSize, MaxNumBlocks>::~Halloc() {
    // shared_ptr handles cleanup when reference count reaches zero
}

}  // namespace hh::halloc