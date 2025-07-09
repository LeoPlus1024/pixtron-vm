#ifndef I_STRING_H
#define I_STRING_H
#include "ptype.h"


extern PStr *pvm_string_intern(const PixtronVM *vm, int32_t length, char *c_str);


extern bool pvm_string_equal(const PStr *a, const PStr *b);


extern uint32_t pvm_string_hash(PStr *str);


extern PStr *pvm_string_new(const char *str, uint32_t len);

extern inline  char *pvm_string_get_data(const PStr *pstr);


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
