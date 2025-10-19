#include <cstddef>
#include <sys/mman.h>
#include <algorithm>
#include "RBTreeDriver.hpp"

#define REQUEST_MEMORY_VIA_MMAP(size) mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)
#define RELEASE_MEMORY_VIA_MUNMAP(ptr, size) munmap(ptr, size)
#define MEMORY_NODE_SIZE sizeof(MemoryNode)
namespace hh::halloc
{
    struct MemoryNode
    {
        MemoryNode *left, *right, *parent;

        /// the bit 63 is for color, bit 62 is for free/used status, rest is size
        std::size_t value;

        MemoryNode *next, *prev;
    };

    std::size_t get_actual_value(std::size_t value)
    {
        return value & ~(3ull << 62);
    }

    std::size_t mark_as_used(std::size_t value)
    {
        return value | (1ull << 62);
    }
    std::size_t mark_as_free(std::size_t value)
    {
        return value & ~(1ull << 62);
    }

    struct Block
    {

        MemoryNode *head;
        RBTreeDriver<MemoryNode> rb_tree;
        std::size_t size;

        Block() : head(nullptr), size(0), rb_tree() {}

        explicit Block(std::size_t bytes)
        {
            size = bytes;
            head = (MemoryNode *)REQUEST_MEMORY_VIA_MMAP(std::max(bytes, MEMORY_NODE_SIZE));
            if (head == MAP_FAILED)
            {
                head = nullptr;
            }
            rb_tree = std::move(RBTreeDriver<MemoryNode>(head));
        }

        Block(Block &&other)
            : head(other.head), size(other.size), rb_tree(std::move(other.rb_tree))
        {
            other.head = nullptr;
            other.size = 0;
        }

        Block &operator=(Block &&other)
        {
            if (this != &other)
            {
                head = other.head;
                size = other.size;
                rb_tree = std::move(other.rb_tree);

                other.head = nullptr;
                other.size = 0;
            }
            return *this;
        }

        MemoryNode *best_fit(std::size_t bytes)
        {
            MemoryNode *node = rb_tree.lower_bound(bytes, [](std::size_t a, std::size_t b)
                                                   { return get_actual_value(a) < get_actual_value(b); });
            return node;
        }
        ~Block()
        {
            if (head)
            {
                RELEASE_MEMORY_VIA_MUNMAP(head, size);
            }
        }

        void *allocate(std::size_t bytes)
        {
        }
        void deallocate(void *ptr, std::size_t bytes)
        {
        }
    };
}
