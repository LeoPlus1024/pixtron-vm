#include "Type.h"

#include <assert.h>
#include <string.h>

extern inline Type PixtronVM_typeof(const VMValue value) {
    if ((value & 0x7FF0000000000000) != 0x7FF0000000000000) {
        return TYPE_DOUBLE;
    }
    return (value >> 48) & 0x0f;
}

extern void inline PixtronVM_to_VMValue(const Variant *variant, uint8_t *value) {
    const Type type = variant->type;
    const uint8_t size = TYPE_SIZE[type];
    memcpy(value, &variant->value, size);
    if (type == TYPE_DOUBLE) {
        return;
    }
    const uint8_t tmp = VM_VALUE_SIZE - 2;
    const uint16_t tag = (0x7FF << 4) | type;
    memcpy(value + tmp, &tag, 2);
}

extern inline void PixtronVM_to_VMDouble(Variant *variant) {
    assert(variant != NULL);
    assert(VM_TYPE_NUMBER(variant->type));
    if (variant->type == TYPE_DOUBLE) {
        return;
    }
    double value;
    switch (variant->type) {
#define HANDLE_CASE(type,field) \
        case type: value = (double)(variant->value.field);  break;
        HANDLE_CASE(TYPE_BYTE, b);
        HANDLE_CASE(TYPE_SHORT, s);
        HANDLE_CASE(TYPE_INT, i);
        HANDLE_CASE(TYPE_LONG, l);
        default: assert(0);
#undef HANDLE_CASE
    }
    variant->value.d = value;
    variant->type = TYPE_DOUBLE;
}

extern inline void PixtronVM_to_VMLong(Variant *variant) {
    assert(variant != NULL);
    assert(VM_TYPE_NUMBER(variant->type));
    if (variant->type == TYPE_LONG) {
        return;
    }
    long value;
    switch (variant->type) {
#define HANDLE_CASE(type,field) \
case type: value = (uint64_t)(variant->value.field);  break;
        HANDLE_CASE(TYPE_BYTE, b);
        HANDLE_CASE(TYPE_SHORT, s);
        HANDLE_CASE(TYPE_INT, i);
        default: assert(0);
#undef HANDLE_CASE
    }
    variant->value.l = value;
    variant->type = TYPE_LONG;
}

extern inline void PixtronVM_negative(Variant *variant) {
    assert(variant != NULL);
    assert(VM_TYPE_NUMBER(variant->type)&&"Negative only support number type.");
    switch (variant->type) {
#define HANDLE_CASE(type,field) \
        case type: variant->value.field = -variant->value.field; break
        HANDLE_CASE(TYPE_BYTE, b);
        HANDLE_CASE(TYPE_SHORT, s);
        HANDLE_CASE(TYPE_INT, i);
        HANDLE_CASE(TYPE_DOUBLE, d);
        HANDLE_CASE(TYPE_LONG, l);
#undef HANDLE_CASE
        default: assert(0&&"Not support number type.");
    }
}
