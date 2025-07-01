#include "bc_reader.h"
#include "itype.h"


extern inline uint8_t pvm_bytecode_read_u8(const RuntimeContext *context) {
    VirtualStackFrame *frame = context->frame;
    const uint32_t pc = frame->pc;
    const uint8_t byte = frame->bytecode[pc];
    frame->pc = pc + 1;
    return byte;
}


extern inline void pvm_bytecode_read_imm(const RuntimeContext *context, const Type type, VMValue *value) {
    const uint8_t size = TYPE_SIZE[type];
    VirtualStackFrame *frame = context->frame;
    const uint32_t pc = frame->pc;
    const uint8_t *buffer = frame->bytecode + pc;
    pvm_load_typed_value_from_buffer(type, value, buffer);
    frame->pc = pc + size;
}

extern inline uint32_t pvm_bytecode_read_u32(const RuntimeContext *context) {
    VirtualStackFrame *frame = context->frame;
    const uint64_t pc = frame->pc;
    const uint8_t *bytecode = frame->bytecode;
    const uint32_t *value = (guint32 *) (bytecode + pc);
    frame->pc = pc + 4;
    return *value;
}

extern inline uint16_t pvm_bytecode_read_u16(const RuntimeContext *context) {
    VirtualStackFrame *frame = context->frame;
    const uint64_t pc = frame->pc;
    const uint8_t *bytecode = frame->bytecode;
    const uint16_t *value = (uint16_t *) (bytecode + pc);
    frame->pc = pc + 2;
    return *value;
}
