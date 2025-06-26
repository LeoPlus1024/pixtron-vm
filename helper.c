#include "helper.h"

#include <dlfcn.h>
#include <glib.h>
#include <stdio.h>
#include "ierror.h"
#include <stdbool.h>
#include <ffi.h>

#include "stack.h"

#ifdef _WIN64
#define LIB_SUFFIX ".dll"
#include <Windows.h>
#elif __APPLE__
#define LIB_SUFFIX ".dylib"
#else
#define LIB_SUFFIX ".so"
#endif

#define DY_SUFFIX_LEN sizeof(LIB_SUFFIX)

extern inline int32_t pvm_get_cstr_len(const char *str) {
    g_assert(str != NULL);
    const size_t len = strlen(str);
    return (int32_t) len + 1;
}


extern inline bool pvm_lookup_native_handle(const Klass *klass, Method *method, GError **error) {
    const char *method_name = method->name;
    const uint64_t len = strlen(method_name) + 1;
    char native_method_name[len];
    snprintf(native_method_name, len, "%s\0", method_name);
    const char *library = method->lib_name;
    if (library == NULL) {
        library = klass->library;
    }
    void *fptr = NULL;
    if (library == NULL) {
        fptr = dlsym(RTLD_DEFAULT, method->name);
    } else {
        const uint64_t length = strlen(library) + DY_SUFFIX_LEN + 4;
        char buf[length];
        snprintf(buf, length, "lib%s%s\0", library, LIB_SUFFIX);
#ifdef _WIN64
        HMODULE handle = LoadLibraryA(buf);
#else
        void *handle = dlopen(buf, RTLD_LAZY);
#endif
        if (handle == NULL) {
#ifdef _WIN64
            DWORD win_err = GetLastError();
            g_set_error(error, KLASS_DOMAIN, LIBRARY_NOT_FOUND, "LoadLibrary(%s) failed (0x%lx)", buf, win_err);
#else
            const char *dl_err = dlerror();
            g_set_error(error, KLASS_DOMAIN, LIBRARY_NOT_FOUND, "dlopen(%s) failed: %s", buf, dl_err);
#endif
            return false;
        }
#ifdef _WIN64
        fptr = GetProcAddress(handle, native_method_name);
#else
        fptr = dlsym(handle, native_method_name);
#endif
        if (fptr == NULL) {
#ifdef _WIN64
            DWORD win_err = GetLastError();
            FreeLibrary(handle);
            g_set_error(error, KLASS_DOMAIN, FUNCTION_NOT_FOUND,"GetProcAddress(%s) failed (0x%lx)", native_method_name, win_err);
#else
            const char *dl_err = dlerror();
            dlclose(handle);
            g_set_error(error, KLASS_DOMAIN, METHOD_NOT_FOUND, "dlsym(%s) failed: %s", native_method_name, dl_err);
#endif
        }
    }
    method->native_handle = fptr;
    return fptr != NULL;
}

extern inline void pvm_ffi_call(RuntimeContext *context, const Method *method) {
    if (!method->native_func) {
        context->throwException(context, "FFI call only native method.");
    }
    const uint16_t argv = method->argv;
    ffi_type *args[argv];
    void *values[argv];

    // Initialize native method params
    for (int i = argv - 1; i >= 0; --i) {
        const MethodParam *param = method->args + i;
        VMValue *operand = pvm_pop_operand(context);
        if (operand->type != param->type) {
            context->throwException(context, "FFI call only one argument that is the same type.");
        }
        switch (param->type) {
            case TYPE_BOOL:
            case TYPE_BYTE:
                args[i] = &ffi_type_sint8;
                values[i] = &(operand->i8);
                break;
            case TYPE_INT:
                args[i] = &ffi_type_sint32;
                values[i] = &(operand->i32);
                break;
            case TYPE_SHORT:
                args[i] = &ffi_type_sshort;
                values[i] = &(operand->i16);
                break;
            case TYPE_DOUBLE:
                args[i] = &ffi_type_double;
                values[i] = &(operand->f64);
                break;
            default:
                args[i] = &ffi_type_pointer;
                values[i] = operand->obj;
        }
    }
    ffi_cif cif;
    const bool success = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, argv, &ffi_type_sint, args) == FFI_OK;
    if (!success) {
        context->throwException(context, "FFI init failed.");
    }
    const Type ret = method->ret;
    void *func_ptr = method->native_handle;
    if (ret != TYPE_VOID) {
        VMValue value;
        value.i64 = 0;
        value.type = ret;
        void *retval = NULL;
        switch (ret) {
            case TYPE_BOOL:
            case TYPE_BYTE:
            case TYPE_INT:
            case TYPE_LONG:
                retval = &value.i64;
                break;
            case TYPE_DOUBLE:
                retval = &value.f64;
                break;
            default:
                retval = &value.obj;
        }
        ffi_call(&cif, func_ptr, retval, values);
        // Push ret value to operand stack
        pvm_push_operand(context, &value);
    } else {
        ffi_call(&cif, func_ptr,NULL, values);
    }
}
