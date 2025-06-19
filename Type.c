#include "Type.h"

#include <assert.h>

extern inline VMValue PixtronVM_CreateValueFromBuffer(const Type type, const uint8_t *buf) {
    VMValue value;
    memcpy(&value, buf, TYPE_SIZE[type]);
    if (type == TYPE_DOUBLE) {
        return value;
    }
    const uint8_t tmp = VM_VALUE_SIZE - 2;
    const uint16_t tag = (0x7FF << 4) | type;
    memcpy(&value + tmp, &tag, 2);
    return value;
}

extern inline void PixtronVM_ConvertToDoubleValue(VMValue *value) {
    if (value->type == TYPE_DOUBLE) {
        return;
    }
    double tmp;
    switch (value->type) {
#define HANDLE_CASE(type,field) \
        case type: tmp = (double)(value->field);  break;
        HANDLE_CASE(TYPE_BYTE, i8)
        HANDLE_CASE(TYPE_SHORT, i16)
        HANDLE_CASE(TYPE_INT, i32)
        HANDLE_CASE(TYPE_LONG, i64)
        default: assert(0);
#undef HANDLE_CASE
    }
    value->f64 = tmp;
    value->type = TYPE_DOUBLE;
}

extern inline void PixtronVM_ConvertToLongValue(VMValue *value) {
    if (value->type == TYPE_LONG) {
        return;
    }
    long tmp;
    switch (value->type) {
#define HANDLE_CASE(type,field) \
case type: tmp = (uint64_t)(value->field);  break;
        HANDLE_CASE(TYPE_BYTE, i8);
        HANDLE_CASE(TYPE_SHORT, i32);
        HANDLE_CASE(TYPE_INT, i64);
        default: assert(0);
#undef HANDLE_CASE
    }
    value->i64 = tmp;
    value->type = TYPE_LONG;
}