#include "itype.h"

#include <assert.h>

extern inline void pvm_value_to_double(VMValue *value) {
    switch (value->type) {
#define HANDLE_CASE(type,field) \
        case type: value->f64 = (double)(value->field);  break;
        HANDLE_CASE(TYPE_INT, i32)
        HANDLE_CASE(TYPE_LONG, i64)
        default: assert(0);
#undef HANDLE_CASE
    }
    value->type = TYPE_DOUBLE;
}

extern inline void pvm_value_to_long(VMValue *value) {
    switch (value->type) {
#define HANDLE_CASE(type,field) \
case type: value->i64 = (int64_t)(value->field);  break;
        HANDLE_CASE(TYPE_SHORT, i32);
        HANDLE_CASE(TYPE_DOUBLE, f64);
        default: assert(0);
#undef HANDLE_CASE
    }
    value->type = TYPE_LONG;
}

extern inline void pvm_value_to_int(VMValue *value) {
    switch (value->type) {
#define HANDLE_CASE(type,field) \
case type: value->i32 = (int32_t)(value->field);  break;
        HANDLE_CASE(TYPE_LONG, i64);
        HANDLE_CASE(TYPE_DOUBLE, f64);
        default: assert(0);
#undef HANDLE_CASE
    }
    value->type = TYPE_INT;
}

extern inline uint8_t pvm_load_typed_value_from_buffer(const Type type, VMValue *value, const uint8_t *buf) {
    const uint8_t size = TYPE_SIZE[type];
    switch (type) {
        case TYPE_BYTE:
        case TYPE_BOOL:
            value->i32 = (int32_t) *(const int8_t *) buf;
            break;
        case TYPE_SHORT:
            value->i32 = *(const int16_t *) buf;
            break;
        default:
            memcpy(value, buf, size);
    }
    value->type = type;
    return size;
}
