#ifndef MALLOC_H
#define MALLOC_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void *malloc(size_t size);

void *realloc(void *ptr, size_t size);

void *calloc(size_t size, size_t cnt);

void free(void *ptr);


#ifdef __cplusplus
}
#endif

#endif /* MALLOC_H */
