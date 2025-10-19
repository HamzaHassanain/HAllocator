#include "basic_alloc.hpp"
#include <iostream>
#include <unistd.h>
#include <climits>
#include <stdexcept>
#include <cstring>
#include <cstdio>

namespace hh::basic_alloc
{
    mem_node *__head = nullptr, *__tail = nullptr;

    inline bool is_free(mem_size_t &size) { return (size & (1ull << 63)); }
    inline void make_free(mem_size_t &size) { size |= (1ull << 63); }
    inline void make_used(mem_size_t &size) { size &= ~(1ull << 63); }
    inline mem_size_t get_size(mem_size_t &size) { return (size & ~(1ull << 63)); }

    inline mem_size_t add(mem_size_t a, mem_size_t b)
    {
        make_used(a);
        make_used(b);
        return a + b;
    }
    inline mem_size_t sub(mem_size_t a, mem_size_t b)
    {
        make_used(a);
        make_used(b);
        return a - b;
    }

    void *sbrk_then_alloc(mem_size_t size)
    {
        mem_node *nxt_node_addr = (mem_node *)sbrk(size + mem_node_size);
        if (nxt_node_addr == (void *)-1)
        {
            throw std::bad_alloc();
        }
        nxt_node_addr->size = size;
        make_used(nxt_node_addr->size);
        if (!__head)
        {
            __head = nxt_node_addr;
            __tail = nxt_node_addr;
        }
        else
        {
            __tail->nxt = nxt_node_addr;
            nxt_node_addr->prv = __tail;
            __tail = nxt_node_addr;
        }
        __tail->nxt = nullptr;

        return (void *)(nxt_node_addr + 1);
    }

    void try_merge(mem_node *nd)
    {
        if (!nd)
            return;

        // Try to merge with the next node
        if (nd->nxt && is_free(nd->nxt->size))
        {
            if (__tail == nd->nxt)
                __tail = nd;

            nd->size = add(nd->size, nd->nxt->size);
            nd->size = add(nd->size, mem_node_size);
            make_free(nd->size);

            nd->nxt = nd->nxt->nxt;
            if (nd->nxt)
                nd->nxt->prv = nd;
        }

        // Try to merge with the previous node
        if (nd->prv && is_free(nd->prv->size))
        {
            if (__tail == nd)
                __tail = nd->prv;

            nd->prv->size = add(nd->prv->size, nd->size);
            nd->prv->size = add(nd->prv->size, mem_node_size);
            make_free(nd->prv->size);

            nd->prv->nxt = nd->nxt;
            if (nd->nxt)
                nd->nxt->prv = nd->prv;
        }

        if (__tail)
            __tail->nxt = nullptr;
    }
    void *free(void *ptr)
    {
        if (!ptr)
            return nullptr;

        mem_node *nd = (mem_node *)ptr - 1;
        make_free(nd->size);

        try_merge(nd);
        return nullptr;
    }

    void shrink_then_align(mem_node *nd, mem_size_t size)
    {
        mem_size_t fragment = sub(nd->size, size);
        if (fragment > MIN_FRAGMENT_SIZE + mem_node_size)
        {
            // Calculate the address of the new node: skip past current node's header and data
            mem_node *new_node = (mem_node *)((char *)(nd + 1) + size);
            new_node->size = sub(fragment, mem_node_size);
            make_free(new_node->size);

            new_node->nxt = nd->nxt;
            new_node->prv = nd;

            if (nd->nxt)
                nd->nxt->prv = new_node;

            nd->size = size;
            make_used(nd->size);

            nd->nxt = new_node;

            if (__tail == nd)
                __tail = new_node;
            else
                try_merge(new_node);
        }
        if (__tail)
            __tail->nxt = nullptr;
    }

    void *try_alloc(mem_size_t size)
    {
        if (!size)
            return nullptr;

        for (auto it = __head; it != nullptr; it = it->nxt)
        {
            if (is_free(it->size) && get_size(it->size) >= size)
            {
                make_used(it->size);
                shrink_then_align(it, size);
                return (void *)(it + 1);
            }
        }
        return sbrk_then_alloc(size);
    }
    void mem_copy(void *dest, const void *src, size_t n)
    {
        if (!dest || !src || n == 0)
            return;

        char *d = (char *)(dest);
        const char *s = (const char *)(src);
        for (size_t i = 0; i < n; ++i)
        {
            d[i] = s[i];
        }
    }
    void *try_realloc(void *ptr, mem_size_t size)
    {
        if (!ptr)
            return try_alloc(size);

        mem_node *nd = (mem_node *)ptr - 1;
        if (get_size(nd->size) >= size)
        {
            shrink_then_align(nd, size);
            return ptr;
        }

        void *new_ptr = try_alloc(size);
        if (new_ptr)
        {
            mem_copy(new_ptr, ptr, get_size(nd->size));
            free(ptr);
        }
        return new_ptr;
    }

    void mem_set(void *ptr, int value, size_t num)
    {
        if (!ptr || num == 0)
            return;

        unsigned char *p = (unsigned char *)ptr;
        for (size_t i = 0; i < num; i++)
        {
            p[i] = (unsigned char)value;
        }
    }

    void *try_calloc(size_t num, size_t size)
    {
        if (num == 0 || size == 0)
            return nullptr;

        // Check for overflow
        if (num > ULLONG_MAX / size)
            return nullptr;

        void *ptr = try_alloc(num * size);
        if (ptr)
        {
            mem_set(ptr, 0, num * size);
        }
        return ptr;
    }

    void alloc_print()
    {
        std::cout << "\n+----------------------------------------------------------------------------------------------------+" << std::endl;
        std::cout << "|                               Memory Allocation Status                                             |" << std::endl;
        std::cout << "+-----------------+----------+---------------+--------------+------------------+---------------------+" << std::endl;
        std::cout << "|     Address     |   Size   |  Total Size   |    Status    |        Prev      |        Next         |" << std::endl;
        std::cout << "+-----------------+----------+---------------+--------------+------------------+---------------------+" << std::endl;

        int block_count = 0;
        mem_size_t total_allocated = 0;
        mem_size_t total_free = 0;

        for (auto it = __head; it != nullptr; it = it->nxt)
        {
            block_count++;
            mem_size_t block_size = get_size(it->size);
            mem_size_t total_size = block_size + mem_node_size;
            bool is_block_free = is_free(it->size);

            if (is_block_free)
            {
                total_free += block_size;
            }
            else
            {
                total_allocated += block_size;
            }

            // Convert addresses to decimal
            unsigned long long addr_decimal = (unsigned long long)(it);
            unsigned long long prev_decimal = it->prv ? (unsigned long long)(it->prv) : 0;
            unsigned long long next_decimal = it->nxt ? (unsigned long long)(it->nxt) : 0;

            printf("| %15llu | %8llu | %13llu | %12s | %16llu | %19llu |\n",
                   addr_decimal,
                   block_size,
                   total_size,
                   is_block_free ? "    FREE    " : "   USED    ",
                   prev_decimal ? prev_decimal : (unsigned long long)(1e13),
                   next_decimal ? next_decimal : (unsigned long long)(1e13));
        }

        std::cout << "+-----------------+----------+---------------+--------------+------------------+---------------------+" << std::endl;
        printf("| Summary: %d blocks | Allocated: %llu bytes | Free: %llu bytes | Total: %llu bytes              |\n",
               block_count, total_allocated, total_free, total_allocated + total_free);
        std::cout << "+----------------------------------------------------------------------------------------------------+" << std::endl;
    }

};