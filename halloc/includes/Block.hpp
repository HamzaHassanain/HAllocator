/**
 * @file Block.hpp
 * @brief Memory block management with Red-Black tree based allocator
 *
 * This file implements a memory block that uses a Red-Black tree to track
 * free memory regions and provides efficient allocation/deallocation.
 */

#pragma once
#include <sys/mman.h>

#include <algorithm>
#include <cstddef>
#include <fstream>

#include "RBTreeDriver.hpp"

/**
 * @def REQUEST_MEMORY_VIA_MMAP
 * @brief Allocates memory from the operating system using mmap
 * @param size Size in bytes to allocate
 * @return Pointer to allocated memory or MAP_FAILED on error
 */
#define REQUEST_MEMORY_VIA_MMAP(size) \
    mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)

/**
 * @def RELEASE_MEMORY_VIA_MUNMAP
 * @brief Releases memory back to the operating system using munmap
 * @param ptr Pointer to memory to release
 * @param size Size in bytes to release
 */
#define RELEASE_MEMORY_VIA_MUNMAP(ptr, size) munmap(ptr, size)

namespace hh::halloc {
/**
 * @struct MemoryNode
 * @brief Node structure for both Red-Black tree and doubly-linked list
 *
 * Each node represents a memory region (either free or allocated).
 * The node serves dual purposes:
 * - Part of a Red-Black tree (via left, right, parent) for efficient searching
 * - Part of a doubly-linked list (via next, prev) for sequential memory management
 *
 * @note Bit 63 of value: Red-Black tree color (1=Red, 0=Black)
 * @note Bit 62 of value: Allocation status (1=Used, 0=Free)
 * @note Bits 0-61 of value: Size of the memory region in bytes
 */
struct MemoryNode {
    MemoryNode* left;    ///< Left child in Red-Black tree
    MemoryNode* right;   ///< Right child in Red-Black tree
    MemoryNode* parent;  ///< Parent node in Red-Black tree

    /**
     * @brief Encoded value containing size, status, and color
     *
     * Layout:
     * - Bit 63: Color (1=Red, 0=Black)
     * - Bit 62: Status (1=Used, 0=Free)
     * - Bits 0-61: Size in bytes
     */
    std::size_t value;

    MemoryNode* next;  ///< Next node in memory sequence (doubly-linked list)
    MemoryNode* prev;  ///< Previous node in memory sequence (doubly-linked list)
};

/**
 * @def MEMORY_NODE_SIZE
 * @brief Size of the MemoryNode metadata structure
 */
#define MEMORY_NODE_SIZE sizeof(MemoryNode)

/**
 * @class Block
 * @brief Manages a contiguous memory block with RB-tree based allocation
 *
 * The Block class provides memory allocation and deallocation within a fixed
 * memory region. It uses:
 * - Red-Black tree for O(log n) best-fit searches among free blocks
 * - Doubly-linked list for O(1) merging of adjacent free blocks
 *
 * Memory is obtained from the OS via mmap and released via munmap.
 * Internal fragmentation is minimized through block splitting and coalescing.
 */
class Block {
    std::size_t size;                  ///< Total block size including metadata
    MemoryNode* head;                  ///< First node in the memory block
    RBTreeDriver<MemoryNode> rb_tree;  ///< Red-Black tree of free nodes

    /**
     * @brief Extracts actual size from encoded value
     * @param value Encoded value with color and status bits
     * @return Size in bytes (bits 0-61)
     */
    std::size_t get_actual_value(std::size_t value);

    /**
     * @brief Marks a memory region as allocated
     * @param value Reference to the node's value field
     * @post Bit 62 is set to 1 (used)
     */
    void mark_as_used(std::size_t& value);

    /**
     * @brief Marks a memory region as free
     * @param value Reference to the node's value field
     * @post Bit 62 is set to 0 (free)
     */
    void mark_as_free(std::size_t& value);

    /**
     * @brief Checks if a memory region is free
     * @param value The node's value field
     * @return true if free (bit 62 is 0), false if used
     */
    bool is_free(const std::size_t& value);

    /**
     * @brief Splits a node and creates remainder as new free node
     *
     * If the node is larger than needed, splits it into two:
     * - First part: allocated to user (size = bytes)
     * - Second part: new free node inserted into RB-tree
     *
     * @param node The node to potentially split
     * @param bytes Size requested by user
     * @post node is marked as used
     * @post If large enough, remainder is added to RB-tree as free node
     */
    void shrink_then_align(MemoryNode* node, std::size_t bytes);

    /**
     * @brief Merges adjacent free blocks
     *
     * Attempts to merge the given node with adjacent free blocks:
     * 1. Forward merge: if next node is free
     * 2. Backward merge: if previous node is free
     *
     * @param node The node to merge with adjacent free blocks
     * @post Adjacent free blocks are coalesced
     * @post Merged node is inserted into RB-tree
     * @post Doubly-linked list is updated
     */
    void coalesce_nodes(MemoryNode* node);

public:
    /**
     * @brief Default constructor - creates invalid block
     * @post size = 0, head = nullptr
     */
    Block();

    /**
     * @brief Constructs a memory block of specified size
     *
     * Allocates memory from OS via mmap and initializes the block
     * with a single large free node.
     *
     * @param bytes Total block size in bytes
     * @throws std::bad_alloc if mmap fails
     * @post Block is initialized with one free node of size (bytes - MEMORY_NODE_SIZE)
     */
    explicit Block(std::size_t bytes);

    /**
     * @brief Move constructor
     * @param other Block to move from
     * @post other is left in valid but empty state
     */
    Block(Block&& other);

    /**
     * @brief Move assignment operator
     * @param other Block to move from
     * @return Reference to this block
     * @post other is left in valid but empty state
     */
    Block& operator=(Block&& other);

    /**
     * @brief Finds best-fit free node for allocation
     *
     * Searches the RB-tree for the smallest free node that can fit
     * the requested size.
     *
     * @param bytes Size in bytes to allocate
     * @return Pointer to best-fit node, or nullptr if no suitable node exists
     * @note Uses RB-tree's lower_bound for O(log n) search
     */
    MemoryNode* best_fit(std::size_t bytes);

    /**
     * @brief Destructor - releases memory back to OS
     * @post Memory is returned via munmap
     */
    ~Block();

    /**
     * @brief Gets total block size
     * @return Size in bytes including all metadata
     */
    std::size_t get_size() const { return size; }

    /**
     * @brief Gets pointer to the head node
     * @return Pointer to first memory node
     */
    void* get_head() const { return head; }

    /**
     * @brief Allocates memory from a specific node
     *
     * Removes node from RB-tree, potentially splits it if too large,
     * and returns pointer to usable memory (after metadata).
     *
     * @param bytes Size in bytes requested
     * @param node The node to allocate from (typically from best_fit)
     * @return Pointer to usable memory (skips MEMORY_NODE_SIZE header)
     * @pre node must be a valid free node from this block
     * @post node is marked as used
     * @post If node was larger, remainder is added as new free node
     */
    void* allocate(std::size_t bytes, MemoryNode* node);

    /**
     * @brief Deallocates previously allocated memory
     *
     * Marks the region as free and attempts to merge with adjacent
     * free blocks to reduce fragmentation.
     *
     * @param ptr Pointer returned from allocate()
     * @param bytes Size parameter (currently unused)
     * @pre ptr must have been returned from allocate() on this block
     * @post Memory is marked as free
     * @post Adjacent free blocks are merged if possible
     * @post Merged block is inserted into RB-tree
     */
    void deallocate(void* ptr, std::size_t bytes);
};
}  // namespace hh::halloc