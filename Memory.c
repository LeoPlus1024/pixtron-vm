//
// Created by leo on 25-5-22.
//

#include "include/Memory.h"
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
