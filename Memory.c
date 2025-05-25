#include "Memory.h"
#include <malloc.h>
#include <stdlib.h>

extern void *PixotronVM_calloc(size_t size) {
    void *ptr = malloc(size);
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
