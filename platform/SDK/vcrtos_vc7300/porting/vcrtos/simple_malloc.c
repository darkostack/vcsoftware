#include <string.h>

#include "malloc.h"

extern void *sbrk(int incr);

void __attribute__((weak)) *malloc(size_t size)
{
    if (size != 0)
    {
        void *ptr = sbrk(size);
        if (ptr != (void*) -1)
        {
            return ptr;
        }
    }
    return NULL;
}

void __attribute__((weak)) *realloc(void *ptr, size_t size)
{
    if (ptr == NULL)
    {
        return malloc(size);
    }
    else if (size == 0)
    {
        free(ptr);
        return NULL;
    }
    else
    {
        void *newptr = malloc(size);
        if (newptr)
        {
            memcpy(newptr, ptr, size);
        }
        return newptr;
    }
}

void __attribute__((weak)) *calloc(size_t size, size_t cnt)
{
    void *mem = malloc(size * cnt);
    if (mem)
    {
        memset(mem, 0, size * cnt);
    }
    return mem;
}

void __attribute__((weak)) free(void *ptr)
{
    (void) ptr;
}
