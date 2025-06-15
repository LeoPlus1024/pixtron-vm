#include "DataSegment.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern inline void PixtronVM_data_segment_set(PixtronVMPtr vm, const uint32_t index, const Type type,
                                              const Variant *variant) {
    // assert(type == variant->type);
    // uint8_t *dst = vm->data + index;
    // const uint8_t size = TYPE_SIZE[type];
    // memcpy(dst, &variant->value, size);
}


extern void inline PixtronVM_data_segment_get(PixtronVMPtr vm, uint32_t offset, Variant *variant) {
    // const uint8_t *dst = vm->data + offset;
    // const uint8_t size = TYPE_SIZE[variant->type];
    // memcpy(&(variant->value), dst, size);
}


extern inline uint8_t PixtronVM_code_segment_u8(PixtronVMPtr vm) {
    // const uint64_t pc = vm->pc;
    // const uint8_t byte = vm->buffer[pc];
    // vm->pc = pc + 1;
    // return byte;
}


extern inline void PixtronVM_code_segment_imm(PixtronVMPtr vm, Variant *variant) {
    // const uint8_t size = TYPE_SIZE[variant->type];
    // const uint64_t pc = vm->pc;
    // if (size == 0 || (size + pc) > vm->size) {
    //     fprintf(stderr, "Opcode data width error.");
    //     exit(-1);
    // }
    // const uint8_t *buffer = vm->buffer;
    // memcpy(&variant->value, &buffer[pc], size);
    // vm->pc = pc + size;
}

extern inline uint32_t PixtronVM_code_segment_u32(PixtronVMPtr vm) {
    // const uint8_t *buffer = vm->buffer;
    // const uint64_t pc = vm->pc;
    // const uint32_t *value = (uint32_t *) (buffer + pc);
    // vm->pc = pc + 4;
    // return *value;
}

extern inline uint16_t PixtronVM_code_segment_u16(PixtronVMPtr vm) {
    // const uint8_t *buffer = vm->buffer;
    // const uint64_t pc = vm->pc;
    // const uint16_t *value = (uint16_t *) (buffer + pc);
    // vm->pc = pc + 2;
    // return *value;
}
