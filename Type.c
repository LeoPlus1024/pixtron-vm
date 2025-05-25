#include "Type.h"

#include <string.h>

extern inline DataType PixtronVM_typeof(const VMValue value) {
    if ((value & 0x7FF0000000000000) != 0x7FF0000000000000) {
        return TYPE_DOUBLE;
    }
    return (value >> 48) & 0x0f;
}

extern void inline PixtronVM_to_VMValue(const Variant *variant, uint8_t *value) {
    const DataType type = variant->type;
    const uint8_t size = TYPE_SIZE[type];
    memcpy(value, &variant->value, size);
    if (type == TYPE_DOUBLE) {
        return;
    }
    const uint8_t tmp = VM_VALUE_SIZE - 2;
    const uint16_t tag = (0x7FF << 4) | type;
    memcpy(value + tmp, &tag, 2);
}
