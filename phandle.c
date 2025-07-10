#include "phandle.h"
#include "pobj.h"
#include "ffi.h"

struct _PHandle {
    Method *cleanup;
    Handle handle;
};

static void pvm_handle_destructor(void *object) {
    if (object == NULL) {
        return;
    }
    PHandle *handle = object;
    const Method *method = handle->cleanup;
    if (method == NULL) {
        return;
    }
    ffi_cif cif;
    ffi_type *arg_type[0] = {&ffi_type_pointer};
    const bool success = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 1, &ffi_type_void, arg_type) == FFI_OK;
    if (!success) {
        const Klass *klass = method->klass;
        g_printerr("Native method <%s.%s> ffi init fail.", pvm_get_symbol_value(klass->name),
                   pvm_get_symbol_value(method->name));
        exit(0);
    }
    void *args[] = {&(handle->handle)};
    ffi_call(&cif, method->native_handle, NULL, args);
}


extern inline PHandle *pvm_handle_new(Handle handle, Method *cleanup) {
    PHandle *phandle = pvm_object_new(sizeof(PHandle), (ObjectDestructor) pvm_handle_destructor, 1);
    phandle->cleanup = cleanup;
    phandle->handle = handle;
    return handle;
}
