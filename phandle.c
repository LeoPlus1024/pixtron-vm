#include "phandle.h"
#include "pobj.h"
#include "pcore.h"

struct _PHandle {
    Method *cleanup;
    Handle handle;
};

static void pvm_handle_destructor(void *object) {
    if (object == NULL) {
        return;
    }
    const PHandle *handle = (PHandle *) object;
    if (handle->cleanup == NULL) {
        return;
    }
    Value value;
    value.type = TYPE_HANDLE;
    value.obj = handle->handle;
    const Value *argv[1] = {&value};

    const CallMethodParam param = {
        handle->cleanup,
        1,
        argv
    };

    pvm_call_method(&param);
}


extern inline PHandle *pvm_handle_new(Handle handle, Method *cleanup) {
    PHandle *phandle = pvm_object_new(sizeof(PHandle), (ObjectDestructor) pvm_handle_destructor, 1);
    phandle->cleanup = cleanup;
    phandle->handle = handle;
    return handle;
}
