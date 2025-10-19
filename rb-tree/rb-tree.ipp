#include "rb-tree.hpp"

namespace hh::rb_tree
{

    void set_color_red(std::size_t &value)
    {
        value |= (1ull << 63);
    }
    void set_color_black(std::size_t &value)
    {
        value &= ~(1ull << 63);
    }

    bool is_red(const std::size_t &value)
    {
        return (value & (1ull << 63));
    }

    bool is_black(const std::size_t &value)
    {
        return !(value & (1ull << 63));
    }

    std::size_t get_value(const std::size_t &value)
    {
        return value & ~(1ull << 63);
    }

    inline bool get_color(const std::size_t &value)
    {
        return is_red(value);
    }

    template <typename rb_node>
    /// Move the Right child up
    void left_rotate(rb_node *&root, rb_node *node)
    {
        rb_node *right_child = node->right;
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

    template <typename rb_node>
    /// Move the Left child up
    void right_rotate(rb_node *&root, rb_node *node)
    {
        rb_node *left_child = node->left;
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

    template <typename rb_node>
    void fix_insert(rb_node *&root, rb_node *z)
    {
        while (z->parent && is_red(z->parent->value))
        {
            if (z->parent == z->parent->parent->left)
            {
                rb_node *y = z->parent->parent->right;
                if (y && is_red(y->value))
                {
                    set_color_black(y->value);
                    set_color_black(z->parent->value);
                    set_color_red(z->parent->parent->value);
                    z = z->parent->parent;
                }
                else
                {
                    if (z == z->parent->right)
                    {
                        z = z->parent;
                        left_rotate(root, z);
                    }
                    set_color_black(z->parent->value);
                    set_color_red(z->parent->parent->value);
                    right_rotate(root, z->parent->parent);
                }
            }
            else
            {
                rb_node *y = z->parent->parent->left;
                if (y && is_red(y->value))
                {
                    set_color_black(z->parent->value);
                    set_color_black(y->value);
                    set_color_red(z->parent->parent->value);
                    z = z->parent->parent;
                }
                else
                {
                    if (z == z->parent->left)
                    {
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

    template <typename rb_node>
    void insert(rb_node *&root, rb_node *new_node)
    {

        rb_node *y = nullptr;
        rb_node *x = root;

        std::size_t new_val = get_value(new_node->value);

        while (x)
        {
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

    template <typename rb_node>
    void transplant(rb_node *&root, rb_node *u, rb_node *v)
    {
        if (!u->parent)
            root = v;
        else if (u == u->parent->left)
            u->parent->left = v;
        else
            u->parent->right = v;

        if (v)
            v->parent = u->parent;
    }

    template <typename rb_node>
    void fix_remove(rb_node *&root, rb_node *x, rb_node *x_parent)
    {
        while (x != root && (x == nullptr || is_black(x->value)))
        {
            if (x == (x_parent ? x_parent->left : nullptr))
            {
                rb_node *w = x_parent->right;
                if (is_red(w->value))
                {
                    set_color_black(w->value);
                    set_color_red(x_parent->value);
                    left_rotate(root, x_parent);
                    w = x_parent->right;
                }
                if ((w->left == nullptr || is_black(w->left->value)) &&
                    (w->right == nullptr || is_black(w->right->value)))
                {
                    set_color_red(w->value);
                    x = x_parent;
                    x_parent = x->parent;
                }
                else
                {
                    if (w->right == nullptr || is_black(w->right->value))
                    {
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
            }
            else
            {
                rb_node *w = x_parent->left;
                if (is_red(w->value))
                {
                    set_color_black(w->value);
                    set_color_red(x_parent->value);
                    right_rotate(root, x_parent);
                    w = x_parent->left;
                }
                if ((w->right == nullptr || is_black(w->right->value)) &&
                    (w->left == nullptr || is_black(w->left->value)))
                {
                    set_color_red(w->value);
                    x = x_parent;
                    x_parent = x->parent;
                }
                else
                {
                    if (w->left == nullptr || is_black(w->left->value))
                    {
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
    template <typename rb_node>
    void remove(rb_node *&root, rb_node *z)
    {
        if (!z or !root)
            return;

        rb_node *y = z;
        rb_node *x = nullptr;
        rb_node *x_parent = nullptr;
        bool is_y_original_black = is_black(y->value);

        if (!z->left)
        {
            x = z->right;
            x_parent = z->parent;
            transplant(root, z, z->right);
        }
        else if (!z->right)
        {
            x = z->left;
            x_parent = z->parent;
            transplant(root, z, z->left);
        }
        else
        {
            // Find successor (minimum in right subtree)
            y = z->right;
            while (y->left)
                y = y->left;

            is_y_original_black = is_black(y->value);
            x = y->right;

            if (y->parent == z)
            {
                // y is the right child of z
                // After transplanting y to z's position, x will be y's right child
                // So x's parent will be y after the transplant
                x_parent = y;
                if (x)
                    x->parent = y;
            }
            else
            {
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

        if (is_y_original_black)
        {
            fix_remove(root, x, x_parent);
        }
    }

    template <typename rb_node>
    rb_node *lower_bound(rb_node *root, std::size_t value, bool (*cmp)(std::size_t, std::size_t))
    {

        if (!root)
            return nullptr;

        auto current = root;
        rb_node *result = nullptr;
        while (current)
        {
            std::size_t curr_val = get_value(current->value);
            if (!cmp(curr_val, value))
            {
                result = current;
                current = current->left;
            }
            else
            {
                current = current->right;
            }
        }
        return result;
    }
}