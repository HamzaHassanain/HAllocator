/**
 * @file Halloc.hpp
 * @brief High-level allocator interface using Red-Black tree for efficient memory management.
 *
 * This file defines the main Halloc allocator class, which provides a type-safe,
 * scalable memory allocator built on BlocksContainer. It uses Red-Black trees for
 * O(log n) best-fit allocation and supports automatic block creation.
 *
 * Default configuration:
 * - Block size: 128 MB
 * - Maximum blocks: 1 (total capacity: 128 MB)
 */

#pragma once

#include <memory>

#include "BlocksContainer.hpp"

const int DEFAULT_BLOCK_SIZE = (128 * 1024 * 1024);  ///< Default block size: 128 MB
const int DEFAULT_MAX_NUM_BLOCKS = 1;                ///< Default max blocks: 1

namespace hh::halloc {
/**
 * @brief Type-safe allocator using Red-Black tree for best-fit allocation.
 *
 * This class provides a high-level allocator interface compatible with STL containers.
 * It manages multiple memory blocks (via BlocksContainer) and provides type-safe
 * allocation/deallocation for objects of type T.
 *
 * Key features:
 * - Best-fit allocation strategy (minimizes fragmentation)
 * - O(log n) allocation and deallocation (Red-Black tree)
 * - Automatic block coalescing (merges adjacent free blocks)
 * - Automatic block creation up to MaxNumBlocks limit
 * - Thread-unsafe (caller must synchronize)
 *
 * @tparam T Type of objects to allocate (default: void for raw bytes)
 * @tparam BlockSize Size of each memory block in bytes (default: 256 MB)
 * @tparam MaxNumBlocks Maximum number of blocks (default: 4)
 *
 * @note Compatible with STL containers via std::allocator_traits
 */
template <typename T = void, int BlockSize = DEFAULT_BLOCK_SIZE,
          int MaxNumBlocks = DEFAULT_MAX_NUM_BLOCKS>
class Halloc {
    // Use shared_ptr so allocator can be copied (required by STL containers)
    std::shared_ptr<BlocksContainer<BlockSize, MaxNumBlocks>>
        blocks;  ///< Underlying multi-block container

public:
    // ==================== C++ Allocator Requirements ====================

    using value_type = T;                    ///< Type of allocated objects
    using pointer = T*;                      ///< Pointer to value_type
    using const_pointer = const T*;          ///< Const pointer to value_type
    using reference = T&;                    ///< Reference to value_type
    using const_reference = const T&;        ///< Const reference to value_type
    using size_type = std::size_t;           ///< Type for sizes
    using difference_type = std::ptrdiff_t;  ///< Type for pointer differences

    /**
     * @brief Rebind allocator to allocate different type U.
     *
     * Required by C++ allocator requirements for container internals
     * (e.g., std::vector needs to allocate internal node structures).
     */
    template <typename U>
    struct rebind {
        using other = Halloc<U, BlockSize, MaxNumBlocks>;
    };

    /**
     * @brief Default constructor - creates allocator with one initial block.
     * @post blocks contains one initialized block of size BlockSize
     */
    Halloc();

    /**
     * @brief Copy constructor - shares underlying BlocksContainer.
     * @param other Allocator to copy from
     * @post this->blocks points to same container as other.blocks
     */
    Halloc(const Halloc& other) : blocks(other.blocks) {}

    /**
     * @brief Rebind copy constructor - shares BlocksContainer across types.
     *
     * Allows containers to create allocators for different types
     * (e.g., vector<int> creates allocators for internal structures).
     *
     * @tparam U Different value_type
     * @param other Allocator of different type to copy from
     */
    template <typename U>
    Halloc(const Halloc<U, BlockSize, MaxNumBlocks>& other) : blocks(other.blocks) {}

    /**
     * @brief Assignment operator.
     * @param other Allocator to assign from
     * @return Reference to this
     */
    Halloc& operator=(const Halloc& other) {
        blocks = other.blocks;
        return *this;
    }

    // Allow rebind copy constructor to access private members
    template <typename U, int BS, int MNB>
    friend class Halloc;

    /**
     * @brief Allocates memory for 'count' objects of type T.
     *
     * Requests count * sizeof(T) bytes from the underlying BlocksContainer.
     * If no suitable block exists and space is available, creates a new block.
     *
     * @param count Number of objects to allocate space for
     * @return Pointer to allocated memory, or nullptr if allocation fails
     *
     * @pre count > 0
     * @post If successful, returned pointer is valid and aligned for T
     * @post If unsuccessful (nullptr), no state was modified
     *
     * @note Allocation fails if:
     *       - count * sizeof(T) > BlockSize
     *       - All blocks full and MaxNumBlocks reached
     * @note Does NOT construct objects (use placement new if needed)
     */
    T* allocate(std::size_t count);

    /**
     * @brief Deallocates memory previously allocated for 'count' objects.
     *
     * Returns memory to the owning block, marks it as free, and attempts to
     * merge with adjacent free blocks to reduce fragmentation.
     *
     * @param ptr Pointer previously returned by allocate()
     * @param count Number of objects (must match allocate() call)
     *
     * @pre ptr != nullptr
     * @pre ptr was returned by this allocator's allocate()
     * @pre count matches the value passed to allocate()
     * @post Memory is freed and merged with adjacent free blocks
     *
     * @note Destroy objects (call destructors manually if needed)
     * @warning Undefined behavior if ptr was not allocated by this allocator
     */
    void deallocate(T* ptr, std::size_t count);

    /**
     * @brief Equality comparison - checks if allocators share same container.
     *
     * Two allocators are equal if they can deallocate each other's allocations,
     * which is true when they share the same BlocksContainer.
     *
     * @param other Another Halloc instance
     * @return true if both share same BlocksContainer, false otherwise
     */
    bool operator==(const Halloc& other) const { return blocks == other.blocks; }

    /**
     * @brief Inequality comparison.
     *
     * @param other Another Halloc instance
     * @return true if allocators don't share same container, false otherwise
     */
    bool operator!=(const Halloc& other) const { return !(*this == other); }

    /**
     * @brief Destructor - releases all blocks back to the OS.
     *
     * Each Block destructor will call munmap to return memory to the operating system.
     *
     * @post All memory blocks are returned to OS
     * @warning Do not use any pointers allocated by this allocator after destruction
     */
    ~Halloc();
};
}  // namespace hh::halloc

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