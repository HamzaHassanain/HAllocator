/**
 * @file RBTreeDriver.hpp
 * @brief Template wrapper class for Red-Black tree operations.
 *
 * This file provides a type-safe wrapper around the generic Red-Black tree implementation
 * defined in rb-tree.hpp. It manages the root pointer and delegates operations to the
 * underlying rb_tree namespace functions.
 */

#pragma once

#include "../../rb-tree/rb-tree.hpp"

namespace hh::halloc
{
    /**
     * @brief Template wrapper for managing a Red-Black tree.
     *
     * This class provides a convenient interface for RB-tree operations while maintaining
     * ownership of the root pointer. It enforces move-only semantics to prevent accidental
     * tree duplication.
     *
     * @tparam T Node type (must have fields: value, left, right, parent)
     *
     * @note This class is move-only (no copy constructor/assignment)
     * @note The node type T must be compatible with hh::rb_tree functions
     */
    template <typename T>
    class RBTreeDriver
    {
    private:
        T *root; ///< Pointer to root node of the RB-tree

        /**
         * @brief Recursively prints the RB-tree structure for debugging.
         *
         * Performs a reverse in-order traversal (right-root-left) to display the tree
         * with depth information and node colors.
         *
         * @param node Current node to print
         * @param depth Current depth in the tree (0 = root)
         *
         * @note Output format: "NODE: <value> | Color: <Red/Black> | Depth: <depth>"
         */
        void print_rb_tree(T *node, int depth)
        {
            if (node == nullptr)
                return;

            std::cout << "NODE: " << hh::rb_tree::get_value(node->value)
                      << " | Color: " << (hh::rb_tree::is_red(node->value) ? "Red" : "Black")
                      << " | Depth: " << depth << std::endl;

            print_rb_tree(node->right, depth + 1);

            print_rb_tree(node->left, depth + 1);
        }

    public:
        /**
         * @brief Default constructor - creates an empty tree.
         * @post root == nullptr
         */
        explicit RBTreeDriver() : root(nullptr) {}

        /**
         * @brief Constructor with existing root node.
         *
         * @param node Pointer to existing RB-tree root
         * @post root == node
         *
         * @note Caller transfers ownership of the tree to this driver
         */
        explicit RBTreeDriver(T *node) : root(node) {}

        /**
         * @brief Copy constructor - deleted (move-only semantics).
         */
        RBTreeDriver(const RBTreeDriver &) = delete;

        /**
         * @brief Copy assignment - deleted (move-only semantics).
         */
        RBTreeDriver &operator=(const RBTreeDriver &) = delete;

        /**
         * @brief Move constructor - transfers ownership of tree.
         *
         * @param other Source driver (will be left with nullptr root)
         * @post this->root == other.root (original value)
         * @post other.root == nullptr
         */
        RBTreeDriver(RBTreeDriver &&other) : root(other.root)
        {
            other.root = nullptr;
        }

        /**
         * @brief Move assignment - transfers ownership of tree.
         *
         * @param other Source driver (will be left with nullptr root)
         * @return Reference to this driver
         * @post this->root == other.root (original value)
         * @post other.root == nullptr
         */
        RBTreeDriver &operator=(RBTreeDriver &&other)
        {
            if (this != &other)
            {
                root = other.root;
                other.root = nullptr;
            }
            return *this;
        }

        /**
         * @brief Inserts a node into the RB-tree.
         *
         * Delegates to hh::rb_tree::insert, which handles:
         * - Binary search tree insertion
         * - Red-Black tree property restoration
         * - Root updates
         *
         * @param node Node to insert (must not already be in tree)
         *
         * @pre node != nullptr
         * @pre node is not already in the tree
         * @post Tree contains node
         * @post RB-tree properties are maintained
         *
         * @note Time complexity: O(log n)
         */
        void insert(T *node)
        {
            hh::rb_tree::insert(root, node);
        }

        /**
         * @brief Removes a node from the RB-tree.
         *
         * Delegates to hh::rb_tree::remove, which handles:
         * - Binary search tree deletion
         * - Red-Black tree property restoration
         * - Root updates
         *
         * @param node Node to remove (must be in tree)
         *
         * @pre node != nullptr
         * @pre node is in the tree
         * @post Tree does not contain node
         * @post RB-tree properties are maintained
         *
         * @note Time complexity: O(log n)
         * @warning Do not call this after modifying node->value (tree uses value for comparisons)
         */
        void remove(T *node)
        {
            hh::rb_tree::remove(root, node);
        }

        /**
         * @brief Finds the smallest node with value >= key using custom comparison.
         *
         * Delegates to hh::rb_tree::lower_bound for efficient search.
         *
         * @param key Search key
         * @param cmp Comparison function: returns true if first >= second
         * @return Pointer to node with smallest value >= key, or nullptr if no such node
         *
         * @pre cmp != nullptr
         * @post Return value is nullptr or points to node in tree
         *
         * @note Time complexity: O(log n)
         * @note Used for best-fit allocation (find smallest free block >= requested size)
         */
        T *lower_bound(std::size_t key, bool (*cmp)(std::size_t, std::size_t))
        {
            return hh::rb_tree::lower_bound(root, key, cmp);
        }

        /**
         * @brief Prints the tree structure to standard output (for debugging).
         *
         * Displays nodes in reverse in-order traversal with depth and color information.
         *
         * @post Tree structure is unchanged
         *
         * @note For debugging purposes only
         */
        void print_tree()
        {
            print_rb_tree(root, 0);
        }
    };
}