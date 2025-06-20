#include "ByteCodeReader.h"

#include "StringUtil.h"

static inline void PixtronVM_ByteCodeIndexOutOfBoundsCheck(RuntimeContext *context,
                                                           const VirtualStackFrame *frame,
                                                           const Method *method,
                                                           const guint size) {
    const guint pc = frame->pc;
    if (pc + size <= method->endOffset) {
        return;
    }
    context->throwException(context, "Bytecode index out of bounds.");
}

extern inline guint8 PixtronVM_ReadByteCodeU8(RuntimeContext *context) {
    VirtualStackFrame *frame = context->frame;
    const guint pc = frame->pc;
    const Method *method = frame->method;
    PixtronVM_ByteCodeIndexOutOfBoundsCheck(context, frame, method, 1);
    const Klass *klass = method->klass;
    const guint8 byte = klass->byteCode[pc];
    frame->pc = pc + 1;
    return byte;
}


extern inline void PixtronVM_ReadByteCodeImm(RuntimeContext *context, VMValue *value) {
    const uint8_t size = TYPE_SIZE[value->type];
    VirtualStackFrame *frame = context->frame;
    const Method *method = frame->method;
    const Klass *klass = method->klass;
    PixtronVM_ByteCodeIndexOutOfBoundsCheck(context, frame, method, size);
    const uint32_t pc = frame->pc;
    const uint8_t *buffer = klass->byteCode + pc;
    memcpy(value, buffer, size);
    frame->pc = pc + size;
}

extern inline guint32 PixtronVM_ReadByteCodeU32(RuntimeContext *context) {
    VirtualStackFrame *frame = context->frame;
    const Method *method = frame->method;
    const Klass *klass = method->klass;
    PixtronVM_ByteCodeIndexOutOfBoundsCheck(context, frame, method, 4);
    const uint64_t pc = frame->pc;
    const uint8_t *byteCode = klass->byteCode;
    const uint32_t *value = (guint32 *) (byteCode + pc);
    frame->pc = pc + 4;
    return *value;
}

extern inline guint16 PixtronVM_ReadByteCodeU16(RuntimeContext *context) {
    VirtualStackFrame *frame = context->frame;
    const Method *method = frame->method;
    const Klass *klass = method->klass;
    PixtronVM_ByteCodeIndexOutOfBoundsCheck(context, frame, method, 2);
    const uint64_t pc = frame->pc;
    const uint8_t *byteCode = klass->byteCode;
    const uint16_t *value = (uint16_t *) (byteCode + pc);
    frame->pc = pc + 2;
    return *value;
}

extern inline char *PixtronVM_ReadByteCodeString(RuntimeContext *context) {
    VirtualStackFrame *frame = context->frame;
    const Method *method = frame->method;
    const Klass *klass = method->klass;
    const uint32_t pc = frame->pc;
    char *str = (char *) klass->byteCode + pc;
    frame->pc = pc + PixtronVM_GetStrFullLen(str);
    return str;
}
