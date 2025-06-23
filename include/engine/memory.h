#ifndef MEMORY_H
#define MEMORY_H
#include <stddef.h>

#define TO_REF(ptr) ((void **)(&(ptr)))
#define CAST_REF(ptr) ((void **)ptr)

/**
 * Allocates memory from the PixtronVM's managed memory pool
 * @param size Number of bytes to allocate
 * @return Pointer to the newly allocated memory block, or NULL if allocation failed
 * @note The allocated memory is automatically zero-initialized
 * @warning The returned pointer must be freed using PixotronVM_free()
 *          to avoid memory leaks
 */
extern void *pvm_mem_calloc(size_t size);

/**
 * Releases memory back to the PixtronVM's memory pool
 * @param ref Double pointer to the memory block to free (will be set to NULL after freeing)
 * @note This function safely handles NULL pointers (no-op)
 * @warning Using standard free() on VM-allocated memory will cause undefined behavior
 */
extern void pvm_mem_free(void **ref);


extern void *pvm_mem_cpy(const void *ptr, size_t size);

#endif //MEMORY_H
