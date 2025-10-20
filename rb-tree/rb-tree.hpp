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

namespace hh::rb_tree {
/**
 * @brief Inserts a new node into the Red-Black tree
 *
 * Inserts the given node into the tree and rebalances to maintain
 * Red-Black tree properties. The node is inserted based on its value
 * (excluding the color bit).
 *
 * @tparam RbNode Node type with left, right, parent pointers and value field
 * @param root Reference to the root pointer of the tree
 * @param new_node Pointer to the node to be inserted (must be allocated)
 *
 * @pre new_node must be properly allocated and initialized
 * @pre new_node->value must have bit 63 available for coloring
 * @post Tree maintains Red-Black properties
 * @post root may be modified if tree structure changes
 */
template <typename RbNode>
void insert(RbNode*& root, RbNode* new_node);

/**
 * @brief Removes a node from the Red-Black tree
 *
 * Removes the specified node from the tree and rebalances to maintain
 * Red-Black tree properties. Does not deallocate the node.
 *
 * @tparam RbNode Node type with left, right, parent pointers and value field
 * @param root Reference to the root pointer of the tree
 * @param node Pointer to the node to be removed
 *
 * @pre node must exist in the tree
 * @pre root must not be null
 * @post Tree maintains Red-Black properties
 * @post root may be modified if tree structure changes
 * @post Node is removed but NOT deallocated
 */
template <typename RbNode>
void remove(RbNode*& root, RbNode* node);

/**
 * @brief Finds the smallest node with value >= key
 *
 * Performs a lower_bound search using a custom comparator function.
 * Returns the node with the smallest value that is not less than the key.
 *
 * @tparam RbNode Node type with left, right, parent pointers and value field
 * @param root Pointer to the root of the tree
 * @param key The search key value
 * @param cmp Comparator function returning true if first arg < second arg
 *
 * @return Pointer to the found node, or nullptr if no such node exists
 *
 * @pre cmp must define a strict weak ordering
 * @post Tree structure remains unchanged
 */
template <typename RbNode>
RbNode* lower_bound(RbNode* root, std::size_t key, bool (*cmp)(std::size_t, std::size_t));
}  // namespace hh::rb_tree

namespace hh::rb_tree {

/**
 * @brief Sets the color of a node to RED
 *
 * Marks the node as red by setting bit 63 of the value field to 1.
 *
 * @param value Reference to the node's value field
 * @post Bit 63 of value is set to 1 (RED)
 */
inline void set_color_red(std::size_t& value) {
    value |= (1ull << 63);
}

/**
 * @brief Sets the color of a node to BLACK
 *
 * Marks the node as black by clearing bit 63 of the value field.
 *
 * @param value Reference to the node's value field
 * @post Bit 63 of value is set to 0 (BLACK)
 */
inline void set_color_black(std::size_t& value) {
    value &= ~(1ull << 63);
}

/**
 * @brief Checks if a node is RED
 *
 * @param value The node's value field
 * @return true if the node is red (bit 63 is set), false otherwise
 */
inline bool is_red(const std::size_t& value) {
    return (value & (1ull << 63));
}

/**
 * @brief Checks if a node is BLACK
 *
 * @param value The node's value field
 * @return true if the node is black (bit 63 is clear), false otherwise
 */
inline bool is_black(const std::size_t& value) {
    return !(value & (1ull << 63));
}

/**
 * @brief Extracts the actual value without the color bit
 *
 * Masks out bit 63 (color bit) and returns the actual stored value.
 *
 * @param value The node's value field including color bit
 * @return The actual value with color bit cleared
 */
inline std::size_t get_value(const std::size_t& value) {
    return value & ~(1ull << 63);
}

/**
 * @brief Gets the color bit value
 *
 * @param value The node's value field
 * @return true if red, false if black
 */
inline bool get_color(const std::size_t& value) {
    return is_red(value);
}

/**
 * @brief Performs a left rotation around a node
 *
 * Rotates the subtree so that the right child moves up to take
 * the position of the current node, and the current node becomes
 * the left child of its former right child.
 *
 * Before:      node              After:    right_child
 *             /    \                       /          \
 *           A   right_child             node          C
 *                /      \               /    \
 *               B        C             A      B
 *
 * @tparam RbNode Node type
 * @param root Reference to the root pointer (may be updated)
 * @param node The node around which to rotate
 *
 * @pre node->right must not be nullptr
 * @post Tree structure is modified, parent pointers updated
 */
template <typename RbNode>
void left_rotate(RbNode*& root, RbNode* node) {
    RbNode* right_child = node->right;
    node->right = right_child->left;

    if (right_child->left)
        right_child->left->parent = node;

    right_child->parent = node->parent;

    if (!node->parent)
        root = right_child;
    else if (node == node->parent->left)
        node->parent->left = right_child;
    else
        node->parent->right = right_child;

    right_child->left = node;
    node->parent = right_child;
}

/**
 * @brief Performs a right rotation around a node
 *
 * Rotates the subtree so that the left child moves up to take
 * the position of the current node, and the current node becomes
 * the right child of its former left child.
 *
 * Before:        node          After:    left_child
 *               /    \                    /         \
 *         left_child  C                  A         node
 *         /      \                                 /    \
 *        A        B                               B      C
 *
 * @tparam RbNode Node type
 * @param root Reference to the root pointer (may be updated)
 * @param node The node around which to rotate
 *
 * @pre node->left must not be nullptr
 * @post Tree structure is modified, parent pointers updated
 */
template <typename RbNode>
void right_rotate(RbNode*& root, RbNode* node) {
    RbNode* left_child = node->left;
    node->left = left_child->right;

    if (left_child->right)
        left_child->right->parent = node;

    left_child->parent = node->parent;

    if (!node->parent)
        root = left_child;
    else if (node == node->parent->left)
        node->parent->left = left_child;
    else
        node->parent->right = left_child;

    left_child->right = node;
    node->parent = left_child;
}

/**
 * @brief Fixes Red-Black tree properties after insertion
 *
 * Restores the Red-Black tree properties that may have been violated
 * during insertion. Handles the following violations:
 * - Red node with red parent (double-red violation)
 *
 * Uses recoloring and rotations to maintain:
 * 1. Root is black
 * 2. No two consecutive red nodes
 * 3. All paths have same number of black nodes
 *
 * @tparam RbNode Node type
 * @param root Reference to the root pointer
 * @param z The newly inserted node to fix violations from
 *
 * @post All Red-Black tree properties are restored
 * @post Root is always black
 */
template <typename RbNode>
void fix_insert(RbNode*& root, RbNode* z) {
    while (z->parent && is_red(z->parent->value)) {
        if (z->parent == z->parent->parent->left) {
            RbNode* y = z->parent->parent->right;
            if (y && is_red(y->value)) {
                set_color_black(y->value);
                set_color_black(z->parent->value);
                set_color_red(z->parent->parent->value);
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    left_rotate(root, z);
                }
                set_color_black(z->parent->value);
                set_color_red(z->parent->parent->value);
                right_rotate(root, z->parent->parent);
            }
        } else {
            RbNode* y = z->parent->parent->left;
            if (y && is_red(y->value)) {
                set_color_black(z->parent->value);
                set_color_black(y->value);
                set_color_red(z->parent->parent->value);
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    right_rotate(root, z);
                }
                set_color_black(z->parent->value);
                set_color_red(z->parent->parent->value);
                left_rotate(root, z->parent->parent);
            }
        }
    }
    set_color_black(root->value);
}

/**
 * @brief Inserts a new node into the Red-Black tree
 *
 * Performs standard BST insertion based on node values (excluding color bit),
 * then calls fix_insert to restore Red-Black properties.
 *
 * Algorithm:
 * 1. Find the correct position using BST search
 * 2. Insert the node as a red leaf
 * 3. Fix any Red-Black violations
 *
 * @tparam RbNode Node type
 * @param root Reference to the root pointer
 * @param new_node The node to insert (must be allocated)
 *
 * @pre new_node is properly allocated
 * @pre new_node->value has bit 63 available for coloring
 * @post Node is inserted and colored red initially
 * @post Red-Black properties are maintained
 * @post Tree remains balanced
 *
 * @note Duplicate values are inserted to the right
 */
template <typename RbNode>
void insert(RbNode*& root, RbNode* new_node) {
    RbNode* y = nullptr;
    RbNode* x = root;

    std::size_t new_val = get_value(new_node->value);

    while (x) {
        y = x;
        std::size_t curr_val = get_value(x->value);
        if (new_val < curr_val)
            x = x->left;
        else
            x = x->right;
    }

    new_node->parent = y;

    if (!y)
        root = new_node;
    else if (new_val < get_value(y->value))
        y->left = new_node;
    else
        y->right = new_node;

    new_node->left = nullptr;
    new_node->right = nullptr;

    set_color_red(new_node->value);
    fix_insert(root, new_node);
}

/**
 * @brief Replaces one subtree with another
 *
 * Helper function that replaces the subtree rooted at node u
 * with the subtree rooted at node v, updating parent pointers.
 *
 * @tparam RbNode Node type
 * @param root Reference to the root pointer
 * @param u Node to be replaced
 * @param v Node to replace with (can be nullptr)
 *
 * @post u's position is taken by v
 * @post Parent pointers are correctly updated
 */
template <typename RbNode>
void transplant(RbNode*& root, RbNode* u, RbNode* v) {
    if (!u->parent)
        root = v;
    else if (u == u->parent->left)
        u->parent->left = v;
    else
        u->parent->right = v;

    if (v)
        v->parent = u->parent;
}

/**
 * @brief Fixes Red-Black tree properties after deletion
 *
 * Restores Red-Black properties after a black node has been removed.
 * Handles the case where removing a black node creates a "double-black"
 * violation where a path has one fewer black node.
 *
 * Cases handled:
 * 1. Sibling is red
 * 2. Sibling is black with two black children
 * 3. Sibling is black with red left child and black right child
 * 4. Sibling is black with red right child
 *
 * @tparam RbNode Node type
 * @param root Reference to the root pointer
 * @param x The node that replaced the deleted node (may be nullptr)
 * @param x_parent Parent of x (needed when x is nullptr)
 *
 * @post Red-Black properties are restored
 * @post All paths have equal black height
 * @post Root is black
 */
template <typename RbNode>
void fix_remove(RbNode*& root, RbNode* x, RbNode* x_parent) {
    while (x != root && (x == nullptr || is_black(x->value))) {
        if (x == (x_parent ? x_parent->left : nullptr)) {
            RbNode* w = x_parent->right;
            if (is_red(w->value)) {
                set_color_black(w->value);
                set_color_red(x_parent->value);
                left_rotate(root, x_parent);
                w = x_parent->right;
            }
            if ((w->left == nullptr || is_black(w->left->value)) &&
                (w->right == nullptr || is_black(w->right->value))) {
                set_color_red(w->value);
                x = x_parent;
                x_parent = x->parent;
            } else {
                if (w->right == nullptr || is_black(w->right->value)) {
                    if (w->left)
                        set_color_black(w->left->value);
                    set_color_red(w->value);
                    right_rotate(root, w);
                    w = x_parent->right;
                }
                // w takes the color of x_parent
                auto parent_color = get_color(x_parent->value);
                if (parent_color)
                    set_color_red(w->value);
                else
                    set_color_black(w->value);
                set_color_black(x_parent->value);
                if (w->right)
                    set_color_black(w->right->value);
                left_rotate(root, x_parent);
                x = root;
            }
        } else {
            RbNode* w = x_parent->left;
            if (is_red(w->value)) {
                set_color_black(w->value);
                set_color_red(x_parent->value);
                right_rotate(root, x_parent);
                w = x_parent->left;
            }
            if ((w->right == nullptr || is_black(w->right->value)) &&
                (w->left == nullptr || is_black(w->left->value))) {
                set_color_red(w->value);
                x = x_parent;
                x_parent = x->parent;
            } else {
                if (w->left == nullptr || is_black(w->left->value)) {
                    if (w->right)
                        set_color_black(w->right->value);
                    set_color_red(w->value);
                    left_rotate(root, w);
                    w = x_parent->left;
                }
                // w takes the color of x_parent
                auto parent_color = get_color(x_parent->value);
                if (parent_color)
                    set_color_red(w->value);
                else
                    set_color_black(w->value);
                set_color_black(x_parent->value);
                if (w->left)
                    set_color_black(w->left->value);
                right_rotate(root, x_parent);
                x = root;
            }
        }
    }
    if (x)
        set_color_black(x->value);
}

/**
 * @brief Removes a node from the Red-Black tree
 *
 * Removes the specified node from the tree while maintaining Red-Black
 * properties. Handles three cases:
 * 1. Node has no left child - replace with right child
 * 2. Node has no right child - replace with left child
 * 3. Node has both children - replace with successor (minimum of right subtree)
 *
 * After removal, if a black node was removed, fix_remove is called to
 * restore Red-Black properties.
 *
 * @tparam RbNode Node type
 * @param root Reference to the root pointer
 * @param z The node to remove
 *
 * @pre z exists in the tree
 * @pre root is not nullptr
 * @post z is removed from the tree structure (but not deallocated)
 * @post Red-Black properties are maintained
 * @post Tree remains balanced
 *
 * @note The node is NOT deallocated, only removed from tree structure
 * @note Caller is responsible for memory management of removed node
 */
template <typename RbNode>
void remove(RbNode*& root, RbNode* z) {
    if (!z or !root)
        return;

    RbNode* y = z;
    RbNode* x = nullptr;
    RbNode* x_parent = nullptr;
    bool is_y_original_black = is_black(y->value);

    if (!z->left) {
        x = z->right;
        x_parent = z->parent;
        transplant(root, z, z->right);
    } else if (!z->right) {
        x = z->left;
        x_parent = z->parent;
        transplant(root, z, z->left);
    } else {
        // Find successor (minimum in right subtree)
        y = z->right;
        while (y->left)
            y = y->left;

        is_y_original_black = is_black(y->value);
        x = y->right;

        if (y->parent == z) {
            // y is the right child of z
            // After transplanting y to z's position, x will be y's right child
            // So x's parent will be y after the transplant
            x_parent = y;
            if (x)
                x->parent = y;
        } else {
            // y is deeper in the tree
            x_parent = y->parent;
            // First, replace y with its right child
            transplant(root, y, y->right);

            // Now connect z's right subtree to y
            y->right = z->right;
            if (y->right)
                y->right->parent = y;
        }

        // Replace z with y
        transplant(root, z, y);
        y->left = z->left;
        if (y->left)
            y->left->parent = y;

        // y takes z's color
        auto z_color = get_color(z->value);
        if (z_color == true)
            set_color_red(y->value);
        else
            set_color_black(y->value);
    }

    if (is_y_original_black) {
        fix_remove(root, x, x_parent);
    }
}

/**
 * @brief Finds the first node with value not less than the search key
 *
 * Performs a binary search to find the smallest node that satisfies
 * !cmp(node_value, key), i.e., the first node where key <= node_value
 * according to the comparator.
 *
 * This is equivalent to std::lower_bound for binary search trees.
 *
 * Algorithm:
 * 1. Start at root and traverse down
 * 2. If key < current_value, move left (might be result)
 * 3. If key >= current_value, move right
 * 4. Return the smallest valid node found
 *
 * @tparam RbNode Node type
 * @param root Pointer to the root of the tree
 * @param value The search key
 * @param cmp Comparator function (returns true if first < second)
 *
 * @return Pointer to the first node >= value, or nullptr if none exists
 *
 * @pre cmp defines a strict weak ordering
 * @post Tree structure is unchanged
 *
 * @note Time complexity: O(log n) for balanced tree
 * @note Returns nullptr if all values are less than the search key
 */
template <typename RbNode>
RbNode* lower_bound(RbNode* root, std::size_t value, bool (*cmp)(std::size_t, std::size_t)) {
    auto current = root;
    RbNode* result = nullptr;
    while (current) {
        std::size_t curr_val = get_value(current->value);
        if (cmp(value, curr_val)) {
            result = current;
            current = current->left;
        } else {
            current = current->right;
        }
    }
    return result;
}
}  // namespace hh::rb_tree
