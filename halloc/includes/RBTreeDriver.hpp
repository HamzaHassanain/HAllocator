#pragma once

#include "../../rb-tree/rb-tree.hpp"

namespace hh::halloc
{
    template <typename T>
    class RBTreeDriver
    {
    private:
        T *root;

    public:
        explicit RBTreeDriver() : root(nullptr) {}
        explicit RBTreeDriver(T *node) : root(node) {}

        // no copy constructors
        RBTreeDriver(const RBTreeDriver &) = delete;
        RBTreeDriver &operator=(const RBTreeDriver &) = delete;
        // only move constructors
        RBTreeDriver(RBTreeDriver &&other) : root(other.root)
        {
            other.root = nullptr;
        }
        RBTreeDriver &operator=(RBTreeDriver &&other)
        {
            if (this != &other)
            {
                root = other.root;
                other.root = nullptr;
            }
            return *this;
        }

        void insert(T *node)
        {
            hh::rb_tree::insert(root, node);
        }
        void remove(T *node)
        {
            hh::rb_tree::remove(root, node);
        }

        T *lower_bound(std::size_t key, bool (*cmp)(std::size_t, std::size_t))
        {
            return hh::rb_tree::lower_bound(root, key, cmp);
        }
    };
}