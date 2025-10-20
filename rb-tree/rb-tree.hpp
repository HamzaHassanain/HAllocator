/**
 * @file rb-tree.hpp
 * @brief Red-Black Tree implementation for memory allocator
 *
 * This is a specialized Red-Black Tree implementation designed specifically
 * for memory allocation purposes. It is NOT a general-purpose RB-tree.
 *
 * @note Users are responsible for node allocation and deallocation.
 *
 * @warning Bit 63 of the value field is RESERVED for Red-Black coloring.
 *
 * Required Node Structure:
 * @code
 * struct your_node {
 *     your_node *left, *right, *parent;
 *     std::size_t value;  // Bit 63 reserved for color
 *     // Your additional data members...
 * };
 * @endcode
 */

#pragma once

#include <cstddef>

namespace hh::rb_tree
{
    /**
     * @brief Inserts a new node into the Red-Black tree
     *
     * Inserts the given node into the tree and rebalances to maintain
     * Red-Black tree properties. The node is inserted based on its value
     * (excluding the color bit).
     *
     * @tparam rb_node Node type with left, right, parent pointers and value field
     * @param root Reference to the root pointer of the tree
     * @param new_node Pointer to the node to be inserted (must be allocated)
     *
     * @pre new_node must be properly allocated and initialized
     * @pre new_node->value must have bit 63 available for coloring
     * @post Tree maintains Red-Black properties
     * @post root may be modified if tree structure changes
     */
    template <typename rb_node>
    void insert(rb_node *&root, rb_node *new_node);

    /**
     * @brief Removes a node from the Red-Black tree
     *
     * Removes the specified node from the tree and rebalances to maintain
     * Red-Black tree properties. Does not deallocate the node.
     *
     * @tparam rb_node Node type with left, right, parent pointers and value field
     * @param root Reference to the root pointer of the tree
     * @param node Pointer to the node to be removed
     *
     * @pre node must exist in the tree
     * @pre root must not be null
     * @post Tree maintains Red-Black properties
     * @post root may be modified if tree structure changes
     * @post Node is removed but NOT deallocated
     */
    template <typename rb_node>
    void remove(rb_node *&root, rb_node *node);

    /**
     * @brief Finds the smallest node with value >= key
     *
     * Performs a lower_bound search using a custom comparator function.
     * Returns the node with the smallest value that is not less than the key.
     *
     * @tparam rb_node Node type with left, right, parent pointers and value field
     * @param root Pointer to the root of the tree
     * @param key The search key value
     * @param cmp Comparator function returning true if first arg < second arg
     *
     * @return Pointer to the found node, or nullptr if no such node exists
     *
     * @pre cmp must define a strict weak ordering
     * @post Tree structure remains unchanged
     */
    template <typename rb_node>
    rb_node *lower_bound(rb_node *root, std::size_t key, bool (*cmp)(std::size_t, std::size_t));
}

#include "rb-tree.ipp"