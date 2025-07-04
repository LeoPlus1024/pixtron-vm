#include "itype.h"


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
