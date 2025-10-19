#include <cstddef>
#include <sys/mman.h>

#define REQUEST_MEMORY_VIA_MMAP(size) mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)
#define RELEASE_MEMORY_VIA_MUNMAP(ptr, size) munmap(ptr, size)

struct Block
{
    void *ptr;
    std::size_t size;

    Block()
    {
        ptr = nullptr;
        size = 0;
    }

    ~Block()
    {
        if (ptr)
        {
            RELEASE_MEMORY_VIA_MUNMAP(ptr, size);
        }
    }

    bool allocate(std::size_t bytes)
    {
        ptr = REQUEST_MEMORY_VIA_MMAP(bytes);
        if (ptr == MAP_FAILED)
        {
            ptr = nullptr;
            return false;
        }
        size = bytes;
        return true;
    }
};
