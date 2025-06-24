#include "helper.h"

#include <dlfcn.h>
#include <glib.h>
#include <stdio.h>
#include "ierror.h"

#ifdef _WIN64
#define LIB_SUFFIX ".dll"
#elif __APPLE__
#define LIB_SUFFIX ".dylib"
#else
#define LIB_SUFFIX ".so"
#endif

extern inline int32_t pvm_get_cstr_len(const char *str) {
    g_assert(str != NULL);
    const size_t len = strlen(str);
    return (int32_t) len + 1;
}


extern inline void *pvm_lookup_native_handle(const Klass *klass, Method *method, GError **error) {
    g_assert(klass!=NULL);
    g_assert(method!=NULL);
    const char *klass_name = klass->name;
    const char *method_name = method->name;
    const uint64_t len = strlen(klass_name) + strlen(method_name) + 2;
    char nativeMethodName[len];
    snprintf(nativeMethodName, len, "%s_%s\0", klass_name, method_name);
    const char *library = method->lib_name;
    if (library == NULL) {
        library = klass->library;
    }
    void *fptr = NULL;
    if (library == NULL) {
        fptr = dlsym(RTLD_DEFAULT, method->name);
    } else {
        const uint64_t length = strlen(library) + strlen(LIB_SUFFIX) + 4;
        char buf[length];
        snprintf(buf, length, "lib%s%s\0", library, LIB_SUFFIX);
        void *handle = dlopen(buf, RTLD_LAZY);
        if (handle == NULL) {
            g_set_error(error, KLASS_DOMAIN, LIBRARY_NOT_FOUND, "Can't find library %s", buf);
            return NULL;
        }
        fptr = dlsym(handle, nativeMethodName);
    }
    if (fptr == NULL) {
        g_set_error(error, KLASS_DOMAIN, METHOD_NOT_FOUND, "Can't find native method %s", method_name);
    }
    method->native_handle = fptr;
    return fptr;
}
