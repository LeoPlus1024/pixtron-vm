#include "Type.h"

#include <assert.h>

extern inline VMValue PixtronVM_CreateValueFromBuffer(const Type type, const uint8_t *buf) {
    VMValue value;
    memcpy(&value, buf, TYPE_SIZE[type]);
    if (type == TYPE_FLOAT) {
        return value;
    }
    const uint8_t tmp = VM_VALUE_SIZE - 2;
    const uint16_t tag = (0x7FF << 4) | type;
    memcpy(&value + tmp, &tag, 2);
    return value;
}

extern inline void PixtronVM_ConvertToDoubleValue(VMValue *value) {
    switch (value->type) {
#define HANDLE_CASE(type,field) \
        case type: value->f64 = (double)(value->field);  break;
        HANDLE_CASE(TYPE_INT, i32)
        HANDLE_CASE(TYPE_LONG, i64)
        default: assert(0);
#undef HANDLE_CASE
    }
}

extern inline void PixtronVM_ConvertToLongValue(VMValue *value) {
    switch (value->type) {
#define HANDLE_CASE(type,field) \
case type: value->i64 = (int64_t)(value->field);  break;
        HANDLE_CASE(TYPE_SHORT, i32);
        HANDLE_CASE(TYPE_FLOAT, f64);
        default: assert(0);
#undef HANDLE_CASE
    }
    value->type = TYPE_LONG;
}

extern inline void PixtronVM_ConvertToIntValue(VMValue *value) {
    switch (value->type) {
#define HANDLE_CASE(type,field) \
case type: value->i32 = (int32_t)(value->field);  break;
        HANDLE_CASE(TYPE_LONG, i64);
        HANDLE_CASE(TYPE_FLOAT, f64);
        default: assert(0);
#undef HANDLE_CASE
    }
    value->type = TYPE_LONG;
}

extern inline Type PixtronVM_GetValueType(const VMValue *value) {
    if ((value->i64 & 0x7FF0000000000000) != 0x7FF0000000000000) {
        return TYPE_FLOAT;
    }
    return (value->i64 >> 48) & 0x0f;
}
