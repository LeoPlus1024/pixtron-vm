#include "pobj.h"

#include <assert.h>
#include <glib.h>
#include "pmem.h"
#include "config.h"

struct _ObjectHeader {
    atomic_uint_least64_t rc;
    ObjectDestructor destructor;
};

extern inline void *pvm_object_new(uint64_t size,
                                   const ObjectDestructor destructor,
                                   const uint64_t init_ref_count) {
    assert(destructor != NULL);
    assert(init_ref_count > 0);
    size += sizeof(ObjectHeader);
    ObjectHeader *header = pvm_mem_calloc(size);
    header->rc = init_ref_count;
    header->destructor = destructor;
    return CONVERT_TO_OBJECT(header);
}

extern inline void pvm_object_refinc(void *object) {
    if (object == NULL) {
        return;
    }
    ObjectHeader *header = GET_OBJECT_HEADER(object);
    atomic_uint_least64_t * counter = &(header->rc);
    atomic_fetch_add(counter, 1);
}

extern inline void pvm_object_refdec(void *object) {
    if (object == NULL) {
        return;
    }
    ObjectHeader *header = GET_OBJECT_HEADER(object);
    atomic_uint_least64_t * counter = &(header->rc);
    const uint64_t pre = atomic_fetch_sub(counter, 1);
    if (pre != 1) {
        return;
    }
#if VM_DEBUG_ENABLE
    g_debug("Prepare to release memory %p", header);
#endif
    header->destructor(object);
    pvm_mem_free(TO_REF(header));
}
