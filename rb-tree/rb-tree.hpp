#pragma once
/*
rb_nodehis is the Reblack-Black rb_noderee implementation header file.

Please NOTE that this is only written for the convenience of implementing a memory allocator.
So, it is NOT A GENERAL PURPOSE RB-TREE IMPLEMENTATION.
You are responsible for creating the Nodes, and Freeing them, etc.
To Use this RB, Your nodes must have a structure similar to this:
value must be an unsigned 64-bit integer type.

THE BIT INDEXED 63 IS RESERVED FOR RED-BLACK COLORING PURPOSES.

struct your_node
{
    your_node *left, *right, *parent;
    std::size_t value;
    // Your data members go here
};
*/

#include <cstddef>

namespace hh::rb_tree
{
    template <typename rb_node>
    void insert(rb_node *&root, rb_node *new_node);
    template <typename rb_node>
    void remove(rb_node *&root, rb_node *node);
    template <typename rb_node>
    rb_node *lower_bound(rb_node *root, std::size_t key, bool (*cmp)(std::size_t, std::size_t));
}

#include "rb-tree.ipp"