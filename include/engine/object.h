#ifndef OBJECT_H
#define OBJECT_H

#include <stdint.h>

#ifdef __APPLE__
#undef __STDC_HOSTED__
#endif

#include <stdatomic.h>

#define CONVERT_TO_OBJECT(ptr) ((void *)(((uint8_t *)ptr) + sizeof(ObjectHeader)))
#define GET_OBJECT_HEADER(ptr) ((ObjectHeader *)((uint8_t *)ptr - sizeof(ObjectHeader)))

typedef void (*ObjectDestructor)(const void *);

typedef struct {
    atomic_uint_least64_t rc;
    ObjectDestructor destructor;
} ObjectHeader;

/**
 * @brief Allocates and initializes a new reference-counted object.
 *
 * Dynamically allocates memory for an object with built-in reference counting.
 * The memory block consists of two contiguous parts:
 *
 *  ┌────────────────────┬────────────────────┐
 *  │   Object Header    │   User Object      │
 *  ├─────────┬──────────┼────────────────────┤
 *  │  rc     │ destruct │  Actual object     │
 *  │ (atomic)| (funcptr)|  data payload      │
 *  └─────────┴──────────┴────────────────────┘
 *  ^                   ^
 *  |                   |
 *  header_start        return value
 *
 * @param size Total size in bytes of the entire allocation (header + object data).
 *             Must be ≥ sizeof(ObjectHeader). Typically calculated as:
 *                 sizeof(MyStruct) + sizeof(ObjectHeader)
 *
 * @param destructor Function pointer to the type-specific destructor. Called when
 *                   refcount reaches 0. Signature: void (*)(void*).
 *                   Pass NULL if no cleanup is needed.
 *
 * @param init_ref_count Initial reference count value. Typically 1 for newly created objects.
 *
 * @return Pointer to the user-accessible object data (points to the memory area
 *         immediately after the header). NULL if allocation failed.
 *
 * @note Memory layout details:
 *  - Header structure (hidden from user):
 *      struct ObjectHeader {
 *          _Atomic uint64_t rc;          // Reference count
 *          ObjectDestructor destructor;   // Cleanup function
 *      };
 *  - Total allocation size = sizeof(ObjectHeader) + user_object_size
 *  - User receives pointer to the start of their object data
 *
 * @warning Do not modify the header directly. Use PixtronVM_Retain/Release for refcounting.
 *          Object must be freed through PixtronVM_Release mechanism.
 */
extern inline void *pvm_object_new(uint64_t size, const ObjectDestructor destructor, uint64_t init_ref_count);

/**
 * @brief Increases the reference count of a managed object.
 *
 * This atomic operation safely increments the object's reference count. Use when:
 *  - Storing the object in a new data structure
 *  - Passing ownership to another execution context
 *  - Extending the object's lifetime beyond current scope
 *
 * @param object Pointer to a valid object created by PixtronVM_NewObject.
 *
 * @note This is a thread-safe operation with acquire-release semantics.
 *
 * @warning Calling with invalid pointers causes undefined behavior.
 *          Always pair with matching PixtronVM_Release calls.
 */
extern inline void pvm_object_refinc(void *object);

/**
 * @brief Decreases the reference count and potentially destroys the object.
 *
 * Atomically decrements the reference count. If the count reaches zero:
 *  1. Calls the registered destructor (if non-NULL)
 *  2. Deallocates the object memory
 *
 * @param object Pointer to a valid object created by PixtronVM_NewObject.
 *
 * @note Last release triggers destruction:
 *        destructor(object) → PixotronVM_free(header)
 *
 * @warning After calling Release, the object pointer becomes invalid immediately
 *          if refcount reached zero. Never access after release.
 *
 * @concurrency Thread-safe: uses atomic operations with full memory barriers.
 */
extern inline void pvm_object_refdec(void *object);

#endif
