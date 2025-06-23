#include "Object.h"
#include <glib.h>
#include "Memory.h"


extern inline void *PixtronVM_NewObject(uint64_t size,
                                        const ObjectDestructor destructor,
                                        const uint64_t initRefCount) {
    g_assert(destructor != NULL);
    g_assert(initRefCount > 0);
    size += sizeof(ObjectHeader);
    ObjectHeader *header = PixotronVM_calloc(size);
    header->rc = initRefCount;
    header->destructor = destructor;
    return CONVERT_TO_OBJECT(header);
}

extern inline void PixtronVM_ObjectRetain(void *object) {
    g_assert(object!=NULL);
    ObjectHeader *header = GET_OBJECT_HEADER(object);
    atomic_uint_least64_t *counter = &(header->rc);
    atomic_fetch_add(counter, 1);
}

extern inline void PixtronVM_ObjectRelease(void *object) {
    g_assert(object!=NULL);
    ObjectHeader *header = GET_OBJECT_HEADER(object);
    atomic_uint_least64_t *counter = &(header->rc);
    const uint64_t pre = atomic_fetch_sub(counter, 1);
    if (pre != 1) {
        return;
    }
    header->destructor(object);
    PixotronVM_free(&object);
}
