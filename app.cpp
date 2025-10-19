#include <bits/stdc++.h>
#include "rb-tree/rb-tree.hpp"

struct TreeNode
{
    TreeNode *left, *right, *parent;
    std::size_t value;

    TreeNode(std::size_t val) : left(nullptr), right(nullptr), parent(nullptr), value(val) {}
};

void print_tree(TreeNode *node, int depth)
{
    if (!node)
        return;

    std::cout << "(" << hh::rb_tree::get_value(node->value) << ") IS " << (hh::rb_tree::is_red(node->value) ? "RED" : "BLACK") << " AT DEPTH " << depth << std::endl;

    print_tree(node->left, depth + 1);
    print_tree(node->right, depth + 1);
}

int main()
{
    TreeNode *root = nullptr;

    std::vector<std::size_t> values = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};
    std::vector<TreeNode *> nodes;
    for (std::size_t val : values)
    {
        TreeNode *node = new TreeNode(val);
        hh::rb_tree::insert(root, node);
        nodes.push_back(node);
    }

    for (int i = 0; i < nodes.size(); i += 3)
    {
        std::cout << "DELETING: " << hh::rb_tree::get_value(nodes[i]->value) << std::endl;
        hh::rb_tree::remove(root, nodes[i]);
    }

    print_tree(root, 0);

    return 0;
}
