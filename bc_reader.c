#include "bc_reader.h"
#include "itype.h"

static inline void pvm_bytecode_index_check(RuntimeContext *context,
                                            const VirtualStackFrame *frame,
                                            const Method *method,
                                            const uint32_t size) {
    const uint32_t pc = frame->pc;
    if (pc + size <= method->end_offset) {
        return;
    }
    context->throw_exception(context, "Bytecode index out of bounds.");
}

extern inline uint8_t pvm_bytecode_read_u8(RuntimeContext *context) {
    VirtualStackFrame *frame = context->frame;
    const uint32_t pc = frame->pc;
    const Method *method = frame->method;
    pvm_bytecode_index_check(context, frame, method, 1);
    const Klass *klass = method->klass;
    const uint8_t byte = klass->bytecode[pc];
    frame->pc = pc + 1;
    return byte;
}


extern inline void pvm_bytecode_read_imm(RuntimeContext *context, const Type type, VMValue *value) {
    const uint8_t size = TYPE_SIZE[type];
    VirtualStackFrame *frame = context->frame;
    const Method *method = frame->method;
    const Klass *klass = method->klass;
    pvm_bytecode_index_check(context, frame, method, size);
    const uint32_t pc = frame->pc;
    const uint8_t *buffer = klass->bytecode + pc;
    pvm_load_typed_value_from_buffer(type, value, buffer);
    frame->pc = pc + size;
}

extern inline guint32 pvm_bytecode_read_u32(RuntimeContext *context) {
    VirtualStackFrame *frame = context->frame;
    const Method *method = frame->method;
    const Klass *klass = method->klass;
    pvm_bytecode_index_check(context, frame, method, 4);
    const uint64_t pc = frame->pc;
    const uint8_t *bytecode = klass->bytecode;
    const uint32_t *value = (guint32 *) (bytecode + pc);
    frame->pc = pc + 4;
    return *value;
}

extern inline uint16_t pvm_bytecode_read_u16(RuntimeContext *context) {
    VirtualStackFrame *frame = context->frame;
    const Method *method = frame->method;
    const Klass *klass = method->klass;
    pvm_bytecode_index_check(context, frame, method, 2);
    const uint64_t pc = frame->pc;
    const uint8_t *bytecode = klass->bytecode;
    const uint16_t *value = (uint16_t *) (bytecode + pc);
    frame->pc = pc + 2;
    return *value;
}
