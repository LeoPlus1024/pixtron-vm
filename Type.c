#include "Type.h"

#include <assert.h>

extern inline VMValue PixtronVM_CreateValueFromBuffer(const Type type, const guint8 *buf) {
    VMValue value = 0;
    memcpy(&value, buf, TYPE_SIZE[type]);
    if (type == TYPE_DOUBLE) {
        return value;
    }
    const uint8_t tmp = VM_VALUE_SIZE - 2;
    const uint16_t tag = (0x7FF << 4) | type;
    memcpy(&value + tmp, &tag, 2);
    return value;
}

extern inline Type PixtronVM_GetValueType(const VMValue value) {
    if ((value & 0x7FF0000000000000) != 0x7FF0000000000000) {
        return TYPE_DOUBLE;
    }
    return (value >> 48) & 0x0f;
}

extern inline void PixtronVM_ConvertValueToBuffer(const Variant *variant, const guint8 *buf) {
    const Type type = variant->type;
    const uint8_t size = TYPE_SIZE[type];
    memcpy(buf, &variant->value, size);
    if (type == TYPE_DOUBLE) {
        return;
    }
    const uint8_t tmp = VM_VALUE_SIZE - 2;
    const uint16_t tag = (0x7FF << 4) | type;
    memcpy(buf + tmp, &tag, 2);
}

extern inline void PixtronVM_ConvertToDoubleValue(Variant *variant) {
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

extern inline void PixtronVM_ConvertToLongValue(Variant *variant) {
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

extern inline void PixtronVM_ConvertValueToVariant(VMValue value, Variant *variant) {
    const Type type = PixtronVM_GetValueType(value);
    if (type == TYPE_DOUBLE) {
        memcpy(&(variant->value), &value, TYPE_SIZE[type]);
    }
    // 对于long型使用48位填充最高位作为符号位
    else if (type == TYPE_LONG) {
        variant->value.l = VLONG_TO_CLONG(value);
    }
    // 对于 byte、short类型直接扩容到int类型
    else {
        variant->value.i = (int32_t) value;
    }
    variant->type = type;
}
