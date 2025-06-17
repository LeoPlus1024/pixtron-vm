#include "ByteCodeReader.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

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


extern inline uint8_t PixtronVM_ReadByteCodeU8(const RuntimeContext *context) {
    VirtualStackFrame *frame = context->frame;
    const guint pc = frame->pc;
    const Method *method = frame->method;
    const Klass *klass = method->klass;
    const uint8_t byte = klass->byteCode[pc];
    frame->pc = pc + 1;
    return byte;
}


extern inline void PixtronVM_ReadByteCodeImm(RuntimeContext *context, Variant *variant) {
    const guint8 size = TYPE_SIZE[variant->type];
    VirtualStackFrame *frame = context->frame;
    const Method *method = frame->method;
    const Klass *klass = method->klass;
    const guint pc = frame->pc;
    if (size == 0 || (size + pc) > method->endOffset) {
        fprintf(stderr, "Opcode data width error.");
        exit(-1);
    }
    const uint8_t *buffer = klass->byteCode + pc;
    memcpy(&variant->value, &buffer[pc], size);
    frame->pc = pc + size;
}

extern inline uint32_t PixtronVM_code_segment_u32(PixtronVMPtr vm) {
    // const uint8_t *buffer = vm->buffer;
    // const uint64_t pc = vm->pc;
    // const uint32_t *value = (uint32_t *) (buffer + pc);
    // vm->pc = pc + 4;
    // return *value;
}

extern inline uint16_t PixtronVM_ReadByteCodeU16(RuntimeContext *context) {
    VirtualStackFrame *frame = context->frame;
    const Method *method = frame->method;
    const Klass *klass = method->klass;
    const uint64_t pc = frame->pc;
    const guint8 *byteCode = klass->byteCode;
    const uint16_t *value = (uint16_t *) (byteCode + pc);
    frame->pc = pc + 2;
    return *value;
}
