#ifndef HELPER_H
#define HELPER_H
#include "itype.h"

/**
 * Calculates the full buffer size needed to store a null-terminated C string.
 *
 * This includes the string's content plus the null terminator. For example:
 *   "Hello" (strlen=5) requires a buffer size of 6.
 *
 * @param str Null-terminated input string (must not be NULL)
 * @return Total buffer size required to store the string including null terminator
 *
 * @note Uses gint return type for GLib compatibility, but note size limitations:
 *       Maximum theoretical string length is G_MAXINT-1 (2^31-2 on 32-bit systems)
 *       due to signed return type. Consider gsize for pure size calculations.
 */
extern int32_t pvm_get_cstr_len(const char *str);

extern inline bool pvm_lookup_native_handle(const Klass *klass, Method *method, GError **error);

/**
 * Invokes a native method via Foreign Function Interface (FFI).
 *
 * This function handles the complete FFI call process including:
 * - Parameter type conversion
 * - Memory management for temporary string copies
 * - FFI call setup and execution
 * - Return value handling
 *
 * @param context Runtime context of the virtual machine
 * @param method Metadata for the native method being called
 */
extern inline void pvm_ffi_call(RuntimeContext *context, const Method *method);

#endif //HELPER_H
