#ifndef ISTRING_H
#define ISTRING_H
#include "itype.h"

typedef struct {
    uint32_t len;
    char *str;
} String;


extern String *pvm_string_const_pool_add(const PixtronVM *vm, const String *str);


extern bool pvm_string_equal(const String *a, const String *b);


extern uint32_t pvm_string_hash(const String *str);


extern String *pvm_string_new(const char *str, uint32_t len);
#endif //ISTRING_H
