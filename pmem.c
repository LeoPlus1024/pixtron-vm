#include "pmem.h"
#ifdef  __APPLE__
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern inline void *pvm_mem_calloc(const size_t size) {
    if (size == 0) {
        return NULL;
    }
    void *ptr = calloc(1, size);
    if (ptr == NULL) {
        fprintf(stderr, "Out of memory.\n");
        exit(-1);
    }
    return ptr;
}

extern inline void pvm_mem_free(void **ref) {
    if (ref == NULL || *ref == NULL) {
        return;
    }
    void *ptr = *ref;
    free(ptr);
    *ref = NULL;
}

extern inline void *pvm_mem_cpy(const void *ptr, const size_t size) {
    if (ptr == NULL || size == 0) {
        return NULL;
    }
    void *dst = pvm_mem_calloc(size);
    memcpy(dst, ptr, size);
    return dst;
}
