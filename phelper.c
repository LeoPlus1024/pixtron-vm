#include "phelper.h"

#include <dlfcn.h>
#include <glib.h>
#include <stdio.h>
#include "perr.h"
#include <stdbool.h>
#include <ffi.h>

#include "pstr.h"
#include "pmem.h"
#include "pstack.h"

#ifdef _WIN64
#define LIB_SUFFIX ".dll"
#include <Windows.h>
#elif __APPLE__
#define LIB_SUFFIX ".dylib"
#else
#define LIB_SUFFIX ".so"
#endif
#include <assert.h>

#include "config.h"

#define DY_SUFFIX_LEN sizeof(LIB_SUFFIX)

extern inline int32_t pvm_get_cstr_len(const char *str) {
    assert(str != NULL);
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
#ifdef _WIN64
        HMODULE hModule = GetModuleHandle(NULL);
        if (!hModule) {
            DWORD win_err = GetLastError();
            g_set_error(error, KLASS_DOMAIN, LIBRARY_NOT_FOUND, "GetModuleHandle failed (0x%lx)", win_err);
        }
        fptr = GetProcAddress(hModule, native_method_name);
        FreeLibrary(handle);
        if (fptr == NULL) {
            g_set_error(error, KLASS_DOMAIN, METHOD_NOT_FOUND , "GetProcAddress(%s) failed (0x%lx)", native_method_name, win_err);
        }
#else
        fptr = dlsym(RTLD_DEFAULT, method->name);
#endif
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
        FreeLibrary(handle);
#else
        fptr = dlsym(handle, native_method_name);
#endif
        if (fptr == NULL) {
#ifdef _WIN64
            DWORD win_err = GetLastError();
            g_set_error(error, KLASS_DOMAIN, METHOD_NOT_FOUND,"GetProcAddress(%s) failed (0x%lx)", native_method_name, win_err);
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
    const uint16_t argv = method->argv;
    ffi_type *arg_types[argv];
    void *values[argv];
    void *copies[argv];

    // Initialize native method params
    for (int i = argv - 1; i >= 0; --i) {
        const MethodParam *param = method->args + i;
        VMValue *operand = pvm_pop_operand(context);
        const Type type = param->type;
#if VM_DEBUG_ENABLE
        if (type != TYPE_OBJECT && operand->type != type) {
            context->throw_exception(context, "FFI call only one argument that is the same type.");
        }
#endif
        switch (type) {
            case TYPE_BOOL:
            case TYPE_BYTE:
                arg_types[i] = &ffi_type_sint8;
                values[i] = &(operand->i8);
                break;
            case TYPE_INT:
                arg_types[i] = &ffi_type_sint32;
                values[i] = &(operand->i32);
                break;
            case TYPE_SHORT:
                arg_types[i] = &ffi_type_sshort;
                values[i] = &(operand->i16);
                break;
            case TYPE_LONG:
                arg_types[i] = &ffi_type_sint64;
                values[i] = &(operand->i64);
                break;
            case TYPE_DOUBLE:
                arg_types[i] = &ffi_type_double;
                values[i] = &(operand->f64);
                break;
            default:
                arg_types[i] = &ffi_type_pointer;
                if (type == TYPE_STRING) {
                    copies[i] = pvm_string_to_cstr(operand->obj);
                } else if (type == TYPE_OBJECT) {
                    copies[i] = operand;
                } else {
                    copies[i] = (operand->obj);
                }
                values[i] = &copies[i];
        }
    }
    ffi_cif cif;
    const Type ret = method->ret;
    ffi_type rtype;
    switch (ret) {
        case TYPE_BOOL:
            rtype = ffi_type_sint8;
            break;
        case TYPE_SHORT:
            rtype = ffi_type_sint16;
            break;
        case TYPE_INT:
            rtype = ffi_type_sint32;
            break;
        case TYPE_LONG:
            rtype = ffi_type_sint64;
            break;
        case TYPE_DOUBLE:
            rtype = ffi_type_double;
            break;
        case TYPE_VOID:
            rtype = ffi_type_void;
            break;
        default:
            rtype = ffi_type_pointer;
    };
    const bool success = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, argv, &rtype, arg_types) == FFI_OK;
    if (!success) {
        context->throw_exception(context, "FFI init failed.");
    }
    void *fptr = method->native_handle;
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
        ffi_call(&cif, fptr, retval, values);
        // Push ret value to operand stack
        pvm_push_operand(context, &value);
    } else {
        ffi_call(&cif, fptr, NULL, values);
    }
    // Free string copies
    for (int i = 0; i < argv; ++i) {
        const MethodParam *param = method->args + i;
        if (param->type != TYPE_STRING) {
            continue;
        }
        pvm_mem_free(TO_REF(copies[i]));
    }
}

extern void pvm_thrown_exception(RuntimeContext *context, char *fmt, ...) {
    const VirtualStackFrame *frame = context->frame;
    va_list vaList;
    va_start(vaList, fmt);
    char *message = g_strdup_vprintf(fmt, vaList);
    va_end(vaList);
    const char *thread_name = g_thread_get_name(g_thread_self());
    GString *str = g_string_new(NULL);
    g_string_printf(str, "Exception in thread \"%s\":%s\n", thread_name, message);
    const VirtualStackFrame *pre = frame;
    uint32_t stack_depth = 0;
    while (pre != NULL) {
        if (stack_depth > VM_EXCEPT_STACK_DEPTH) {
            break;
        }
        const Method *method = pre->method;
        const Klass *klass = method->klass;
        g_string_append_printf(str, "   at %s.%s(%s)\n", klass->name, method->name, klass->file);
        pre = pre->pre;
        stack_depth++;
    }
    char *except_stacks = g_string_free(str, FALSE);
    g_printerr(except_stacks);
    g_free(except_stacks);
    g_free(message);
    g_thread_self();
    g_thread_exit(NULL);
}
