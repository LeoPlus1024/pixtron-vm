#include "DataSegment.h"

#include <assert.h>
#include <string.h>

extern inline void PixtronVM_data_segment_set(PixtronVMPtr vm, const uint32_t *offset, DataType type,
                                              const Variant *variant) {
    assert(type == variant->type);
    const uint32_t index = *offset;
    uint8_t *dst = vm->data + index;
    const uint8_t size = TYPE_SIZE[type];
    memcpy(dst, &variant->value, size);
}


extern void inline PixtronVM_data_segment_get(PixtronVMPtr vm, const uint32_t *offset, Variant *variant) {
    const uint32_t index = *offset;
    const uint8_t *dst = vm->data + index;
    const uint8_t size = TYPE_SIZE[variant->type];
    memcpy(&(variant->value), dst, size);
}
