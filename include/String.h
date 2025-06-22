#ifndef STRING_H
#define STRING_H
#include <stdint.h>
#include "Type.h"

#define g_string_hash0(ptr) (PixtronVM_StringHash(ptr))

typedef struct {
    uint32_t len;
    char *str;
} String;


extern String *PixtronVM_StringCheckPool(const PixtronVM *vm, const String *str);


extern bool PixtronVM_StringEqual(const String *a, const String *b);


extern uint32_t PixtronVM_StringHash(const String *str);


extern String *PixtronVM_StringNew(const char *str, uint32_t len);

#endif //STRING_H
