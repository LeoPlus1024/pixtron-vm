#include "Memory.h"
#ifdef  __APPLE__
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif
#include <stdio.h>
#include <stdlib.h>

extern void *PixotronVM_calloc(const size_t size) {
    void *ptr = calloc(1, size);
    if (ptr == NULL) {
        fprintf(stderr, "Out of memory.\n");
        exit(-1);
    }
    return ptr;
}

extern void PixotronVM_free(void **ref) {
    if (ref == NULL || *ref == NULL) {
        return;
    }
    void *ptr = *ref;
    free(ptr);
    *ref = NULL;
}
