#pragma once
#include <iostream>
#include <unistd.h>
#include <climits>
#include <stdexcept>
#include <cstring>
#include <cstdio>

namespace hh_basic_alloc
{
    using mem_size_t = unsigned long long;
    constexpr mem_size_t MIN_FRAGMENT_SIZE = 32;
    constexpr mem_size_t BLOCK_SIZE = 4096;

    struct mem_node
    {
        mem_node *nxt, *prv;
        mem_size_t size;
    };

    const mem_size_t mem_node_size = sizeof(mem_node);

    bool is_free(mem_size_t &size);
    void make_free(mem_size_t &size);
    void make_used(mem_size_t &size);
    mem_size_t get_size(mem_size_t &size);

    mem_size_t add(mem_size_t a, mem_size_t b);
    mem_size_t sub(mem_size_t a, mem_size_t b);

    extern mem_node *__head, *__tail;

    void *sbrk_then_alloc(mem_size_t size);

    void try_merge(mem_node *nd);
    void *free(void *ptr);

    void shrink_then_align(mem_node *nd, mem_size_t size);

    void *try_alloc(mem_size_t size);
    void mem_copy(void *dest, const void *src, size_t n);

    void *try_realloc(void *ptr, mem_size_t size);

    void mem_set(void *ptr, int value, size_t num);

    void *try_calloc(size_t num, size_t size);

    void alloc_print();
};