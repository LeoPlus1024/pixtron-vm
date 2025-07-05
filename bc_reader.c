#include "bc_reader.h"


extern inline int8_t pvm_bytecode_read_int8(const RuntimeContext *context) {
    VirtualStackFrame *frame = context->frame;
    const uint32_t pc = frame->pc;
    const int8_t byte = (int8_t) frame->bytecode[pc];
    frame->pc = pc + 1;
    return byte;
}


extern inline int32_t pvm_bytecode_read_int32(const RuntimeContext *context) {
    VirtualStackFrame *frame = context->frame;
    const uint64_t pc = frame->pc;
    const uint8_t *bytecode = frame->bytecode;
    const int32_t *value = (int32_t *) (bytecode + pc);
    frame->pc = pc + 4;
    return *value;
}

extern inline int16_t pvm_bytecode_read_int16(const RuntimeContext *context) {
    VirtualStackFrame *frame = context->frame;
    const uint64_t pc = frame->pc;
    const uint8_t *bytecode = frame->bytecode;
    const int16_t *value = (int16_t *) (bytecode + pc);
    frame->pc = pc + 2;
    return *value;
}

extern inline int64_t pvm_bytecode_read_int64(const RuntimeContext *context) {
    VirtualStackFrame *frame = context->frame;
    const uint64_t pc = frame->pc;
    const uint8_t *bytecode = frame->bytecode;
    const int64_t *value = (int64_t *) (bytecode + pc);
    frame->pc = pc + 8;
    return *value;
}

extern inline double pvm_bytecode_read_f64(const RuntimeContext *context) {
    VirtualStackFrame *frame = context->frame;
    const uint64_t pc = frame->pc;
    const uint8_t *bytecode = frame->bytecode;
    const double *value = (double *) (bytecode + pc);
    frame->pc = pc + 8;
    return *value;
}
