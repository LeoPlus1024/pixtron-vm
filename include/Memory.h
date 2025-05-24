#ifndef MEMORY_H
#define MEMORY_H
#include <stddef.h>

#define TO_REF(ptr) ((void **)(&(ptr)))
#define CAST_REF(ptr) ((void **)ptr)

extern void *PixotronVM_calloc(size_t size);


extern void PixotronVM_free(void **ref);

#endif //MEMORY_H
