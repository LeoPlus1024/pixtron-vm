#include "helper.h"

#include <dlfcn.h>
#include <glib.h>
#include <stdio.h>
#include "ierror.h"
#include <stdbool.h>

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
    const char *klass_name = klass->name;
    const char *method_name = method->name;
    const uint64_t len = strlen(klass_name) + strlen(method_name) + 2;
    char native_method_name[len];
    snprintf(native_method_name, len, "%s_%s\0", klass_name, method_name);
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
