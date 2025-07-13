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
#include "phandle.h"

#define DY_SUFFIX_LEN sizeof(LIB_SUFFIX)

extern inline int32_t pvm_get_cstr_len(const char *str) {
    assert(str != NULL);
    const size_t len = strlen(str);
    return (int32_t) len + 1;
}


extern inline bool pvm_lookup_native_handle(const Klass *klass, Method *method, GError **error) {
    const Symbol *symbol = method->name;
    const Symbol *lib_symbol = klass->library;
    const char *method_name = pvm_get_symbol_value(symbol);

    const char *library = NULL;
    if (lib_symbol != NULL) {
        library = pvm_get_symbol_value(lib_symbol);
    }
    void *fptr = NULL;
    if (lib_symbol == NULL) {
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
        fptr = dlsym(RTLD_DEFAULT, method_name);
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
            g_set_error(error, KLASS_DOMAIN, LIBRARY_NOT_FOUND, dl_err);
#endif
            return false;
        }
#ifdef _WIN64
        fptr = GetProcAddress(handle, native_method_name);
        FreeLibrary(handle);
#else
        fptr = dlsym(handle, method_name);
#endif
        if (fptr == NULL) {
#ifdef _WIN64
            DWORD win_err = GetLastError();
            g_set_error(error, KLASS_DOMAIN, METHOD_NOT_FOUND,"GetProcAddress(%s) failed (0x%lx)", native_method_name, win_err);
#else
            const char *dl_err = dlerror();
            dlclose(handle);
            g_set_error(error, KLASS_DOMAIN, METHOD_NOT_FOUND, "dlsym(%s) failed: %s", method_name, dl_err);
#endif
        }
    }
    if (fptr == NULL) {
        g_set_error(error, KLASS_DOMAIN, METHOD_NOT_FOUND, "Native method <%s.%s> linker fail.",
                    pvm_get_symbol_value(klass->name), method_name);
    }
    method->native_handle = fptr;
    return fptr != NULL;
}

extern inline void pvm_ffi_call(RuntimeContext *context, const Method *method) {
    const uint16_t argc = method->argc;
    ffi_type *arg_types[argc];
    void *values[argc];
    void *copies[argc];

    const bool raw_str = method->raw_str;
    // Initialize native method params
    for (int i = argc - 1; i >= 0; --i) {
        const MethodParam *param = method->argv + i;
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
                    copies[i] = raw_str ? pvm_string_get_data(operand->obj) : pvm_string_to_cstr(operand->obj);
                } else if (type == TYPE_OBJECT) {
                    copies[i] = operand;
                } else if (type == TYPE_HANDLE) {
                    copies[i] = pvm_handle_get_data(operand->obj);
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
    const bool success = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, argc, &rtype, arg_types) == FFI_OK;
    if (!success) {
        context->throw_exception(context, "FFI init failed.");
    }
    void *fptr = method->native_handle;
    if (ret != TYPE_VOID) {
        VMValue *value = pvm_next_operand(context);
        value->type = ret;
        if (ret == TYPE_HANDLE) {
            ffi_call(&cif, fptr, &(value->obj), values);
            value->obj = pvm_handle_new(value->obj, method->m_cleanup);
        } else {
            switch (ret) {
                case TYPE_BOOL:
                case TYPE_BYTE:
                case TYPE_INT:
                case TYPE_LONG:
                    value->i64 = 0;
                    ffi_call(&cif, fptr, &value->i64, values);
                    break;
                case TYPE_DOUBLE:
                    ffi_call(&cif, fptr, &value->f64, values);
                    break;
                default:
                    ffi_call(&cif, fptr, &value->obj, values);
            }
        }
    } else {
        ffi_call(&cif, fptr, NULL, values);
    }
    // If raw_str will skip copies string free
    if (raw_str) {
        return;
    }
    // Free string copies
    for (int i = 0; i < argc; ++i) {
        const MethodParam *param = method->argv + i;
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
        g_string_append_printf(str, "   at %s.%s(%s)\n",
                               pvm_get_symbol_value(klass->name),
                               pvm_get_symbol_value(method->name),
                               pvm_get_symbol_value(klass->file));
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
