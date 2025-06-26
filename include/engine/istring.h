#ifndef I_STRING_H
#define I_STRING_H
#include "itype.h"

typedef struct {
    uint32_t len;
    char *str;
} String;


extern String *pvm_string_const_pool_add(const PixtronVM *vm, const String *str);


extern bool pvm_string_equal(const String *a, const String *b);


extern uint32_t pvm_string_hash(const String *str);


extern String *pvm_string_new(const char *str, uint32_t len);


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
extern char *pvm_string_to_cstr(const String *str);

#endif //I_STRING_H
