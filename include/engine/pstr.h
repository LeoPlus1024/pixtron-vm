#ifndef I_STRING_H
#define I_STRING_H
#include "ptype.h"

typedef struct {
    uint32_t len;
    char *str;
} PStr;


extern PStr *pvm_string_const_pool_add(const PixtronVM *vm, const PStr *str);


extern bool pvm_string_equal(const PStr *a, const PStr *b);


extern uint32_t pvm_string_hash(const PStr *str);


extern PStr *pvm_string_new(const char *str, uint32_t len);


/**
 * @brief Converts a VM String to a C-style string
 *
 * This function converts a VM  String structure into
 * a standard null-terminated C string. It allocates new memory and copies the
 * string content, ensuring proper null termination.
 *
 * @param str Pointer to the PVM String structure containing the string data
 *            - Returns NULL if input is NULL or string length is 0
 *
 * @return Pointer to newly allocated C-style string
 *         - Caller is responsible for memory deallocation
 *         - Returns NULL on failure
 */
extern char *pvm_string_to_cstr(const PStr *str);

#endif //I_STRING_H
