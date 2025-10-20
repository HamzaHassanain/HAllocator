/**
 * @file Block.ipp
 * @brief Implementation of Block memory allocator
 */

#include <sys/mman.h>

#include <algorithm>
#include <cstddef>
#include <fstream>

#include "../includes/Block.hpp"
#include "../includes/RBTreeDriver.hpp"

namespace hh::halloc {

inline std::size_t Block::get_actual_value(std::size_t value) {
    // Clear bits 62-63 (status and color), keep bits 0-61 (size)
    return value & ~(3ull << 62);
}

inline void Block::mark_as_used(std::size_t& value) {
    // Set bit 62 to indicate allocated/used
    value |= (1ull << 62);
}

inline void Block::mark_as_free(std::size_t& value) {
    // Clear bit 62 to indicate free
    value &= ~(1ull << 62);
}

inline bool Block::is_free(const std::size_t& value) {
    // Check if bit 62 is clear (free)
    return !(value & (1ull << 62));
}

inline Block::Block() : size(0), head(nullptr), rb_tree() {}

inline Block::Block(std::size_t bytes) {
    size = bytes;
    head = (MemoryNode*)REQUEST_MEMORY_VIA_MMAP(bytes);

    if (head == MAP_FAILED) {
        head = nullptr;
        throw std::bad_alloc();
    }

    // Initialize the single free node covering the entire block
    head->value = bytes - MEMORY_NODE_SIZE;
    mark_as_free(head->value);

    // Initialize linked list pointers
    head->next = nullptr;
    head->prev = nullptr;

    // Initialize RB-tree pointers
    head->left = nullptr;
    head->right = nullptr;
    head->parent = nullptr;

    // Insert into RB-tree
    rb_tree = RBTreeDriver<MemoryNode>{head};
}

inline Block::Block(Block&& other)
    : size(other.size), head(other.head), rb_tree(std::move(other.rb_tree)) {
    other.head = nullptr;
    other.size = 0;
}

inline Block& Block::operator=(Block&& other) {
    if (this != &other) {
        head = other.head;
        size = other.size;
        rb_tree = std::move(other.rb_tree);

        other.head = nullptr;
        other.size = 0;
    }
    return *this;
}

inline MemoryNode* Block::best_fit(std::size_t bytes) {
    // Find smallest free node >= bytes using RB-tree
    MemoryNode* node =
        rb_tree.lower_bound(bytes, [](std::size_t a, std::size_t b) { return (a) <= (b); });
    return node;
}

/**
 * @brief Allocates memory from a specific free node.
 *
 * This function performs the final allocation step after best_fit has found a suitable node:
 * 1. Removes the node from the RB-tree of free blocks
 * 2. Splits the node if it's larger than needed (via shrink_then_align)
 * 3. Marks the node as used
 * 4. Returns pointer to usable memory (after metadata)
 *
 * @param bytes Number of bytes requested by user (excluding metadata)
 * @param node Free node to allocate from (must be large enough)
 * @return void* Pointer to usable memory area (metadata already skipped)
 *
 * @pre node != nullptr
 * @pre is_free(node->value) == true
 * @pre get_actual_value(node->value) >= bytes
 * @post is_free(node->value) == false (node marked as used)
 * @post If node was split, a new free node exists in RB-tree
 */
inline void* Block::allocate(std::size_t bytes, MemoryNode* node) {
    // Calculate pointer to usable memory (skip metadata)
    void* actual_mem = (void*)((char*)node + MEMORY_NODE_SIZE);

    // Remove from RB-tree (will be marked as used)
    rb_tree.remove(node);

    // Split node if large enough, mark as used
    shrink_then_align(node, bytes);

    return actual_mem;
}

/**
 * @brief Deallocates previously allocated memory and merges with adjacent free blocks.
 *
 * This function reverses the allocation:
 * 1. Converts user pointer back to MemoryNode pointer
 * 2. Marks the node as free
 * 3. Attempts to merge with adjacent free blocks (coalescing)
 * 4. Inserts the (possibly merged) node into RB-tree
 *
 * Coalescing reduces fragmentation by combining adjacent free blocks into larger blocks.
 *
 * @param ptr Pointer previously returned by allocate() (not the MemoryNode pointer)
 * @param bytes Size parameter (unused, for interface compatibility)
 *
 * @pre ptr != nullptr
 * @pre ptr was previously returned by allocate()
 * @pre The block containing this node has not been destroyed
 * @post Node is marked as free
 * @post Node is inserted into RB-tree (possibly merged with neighbors)
 * @post Adjacent free blocks are coalesced if possible
 */
inline void Block::deallocate(void* ptr, [[maybe_unused]] std::size_t bytes) {
    MemoryNode* node = (MemoryNode*)((char*)ptr - MEMORY_NODE_SIZE);

    mark_as_free(node->value);

    // Merge with adjacent free blocks and insert into RB-tree
    coalesce_nodes(node);
}

/**
 * @brief Splits a node if it's significantly larger than requested size.
 *
 * Algorithm:
 * 1. Check if remainder after allocation is large enough (>= MEMORY_NODE_SIZE + 1)
 * 2. If yes:
 *    a. Create new MemoryNode in the remainder space
 *    b. Initialize new node's metadata (size, RB-tree pointers, linked-list pointers)
 *    c. Update doubly-linked list to insert new node after current
 *    d. Shrink current node's size to requested bytes
 *    e. Insert new free node into RB-tree
 * 3. Mark current node as used
 *
 * This prevents internal fragmentation by returning excess memory to the free pool.
 *
 * @param node Node to potentially split (already removed from RB-tree)
 * @param bytes Requested allocation size (excluding metadata)
 *
 * @pre node != nullptr
 * @pre node is not in RB-tree (must be removed before calling)
 * @pre get_actual_value(node->value) >= bytes
 * @post node->value == bytes (or original size if no split occurred)
 * @post is_free(node->value) == false
 * @post If split occurred, a new free node exists in RB-tree and linked list
 *
 * @note Minimum split size: MEMORY_NODE_SIZE + 1 (metadata + at least 1 byte)
 */
inline void Block::shrink_then_align(MemoryNode* node, std::size_t bytes) {
    std::size_t node_size = get_actual_value(node->value);

    // Split only if remainder is large enough for a new node
    if (node_size >= bytes + MEMORY_NODE_SIZE + 1ull) {
        // Create new free node in the remainder space
        MemoryNode* new_node = (MemoryNode*)((unsigned char*)node + MEMORY_NODE_SIZE + bytes);
        std::size_t new_node_size = node_size - bytes - MEMORY_NODE_SIZE;

        mark_as_free(new_node_size);

        new_node->value = new_node_size;

        // Initialize RB-tree pointers
        new_node->left = nullptr;
        new_node->right = nullptr;
        new_node->parent = nullptr;

        // Update doubly-linked list
        new_node->next = node->next;
        new_node->prev = node;

        if (node->next) {
            node->next->prev = new_node;
        }

        node->next = new_node;
        node->value = bytes;

        // Insert remainder into RB-tree as free node
        rb_tree.insert(new_node);
    }

    // Mark current node as used
    mark_as_used(node->value);
}

/**
 * @brief Attempts to merge (coalesce) a free node with adjacent free blocks.
 *
 * Algorithm:
 * 1. Forward merge: If next node exists and is free:
 *    a. Remove next node from RB-tree (critical: do this BEFORE modifying)
 *    b. Add next node's size + metadata to current node's size
 *    c. Update linked list to skip next node
 *    d. Update next->next->prev if it exists
 *
 * 2. Backward merge: If previous node exists and is free:
 *    a. Remove previous node from RB-tree (critical: do this BEFORE modifying)
 *    b. Add current node's size + metadata to previous node's size
 *    c. Update linked list to skip current node
 *    d. Update current->next->prev if it exists
 *    e. Set current node pointer to previous node
 *
 * 3. Insert the (possibly merged) node into RB-tree
 *
 * This function reduces fragmentation by combining adjacent free blocks.
 *
 * @param node Free node to merge with neighbors
 *
 * @pre node != nullptr
 * @pre is_free(node->value) == true
 * @pre node is NOT in RB-tree yet
 * @post node (or merged node) is inserted into RB-tree
 * @post Adjacent free blocks are merged if they existed
 * @post Linked list is updated to reflect any merges
 */
inline void Block::coalesce_nodes(MemoryNode* node) {
    // Forward merge: merge with next node if it's free
    if (node->next && is_free(node->next->value)) {
        MemoryNode* next_node = node->next;

        rb_tree.remove(next_node);

        node->value =
            get_actual_value(node->value) + get_actual_value(next_node->value) + MEMORY_NODE_SIZE;

        mark_as_free(node->value);

        // Update linked list
        node->next = next_node->next;

        if (node->next) {
            node->next->prev = node;
        }
    }

    // Backward merge: merge with previous node if it's free
    if (node->prev && is_free(node->prev->value)) {
        MemoryNode* prev_node = node->prev;

        rb_tree.remove(prev_node);

        prev_node->value =
            get_actual_value(prev_node->value) + get_actual_value(node->value) + MEMORY_NODE_SIZE;

        mark_as_free(prev_node->value);

        // Update linked list
        prev_node->next = node->next;

        if (prev_node->next) {
            prev_node->next->prev = prev_node;
        }

        // Continue with merged node
        node = prev_node;
    }

    // Insert merged node into RB-tree
    rb_tree.insert(node);
}

/**
 * @brief Destructor - releases the entire memory block back to the OS.
 *
 * Uses munmap to return the entire mmap'd region to the operating system.
 * This deallocates all memory nodes at once, regardless of individual allocation status.
 *
 * @post All memory in this Block is returned to OS
 * @post head pointer is invalid after this call
 */
inline Block::~Block() {
    if (head) {
        RELEASE_MEMORY_VIA_MUNMAP(head, size);
    }
}

};  // namespace hh::halloc
