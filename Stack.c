#include "Stack.h"

#include <assert.h>
#include <Config.h>
#include <stdio.h>
#include <stdlib.h>
#include "Memory.h"

static inline VirtualStackFrame *PixtronVM_IPushStackFrame(RuntimeContext *context, const Method *method,
                                                           const uint16_t argv) {
    if (argv != method->argv) {
        context->throwException(context, "Invalid number of arguments expect argv is %d but is %d", method->argv, argv);
    }
    const uint32_t stackDepth = context->stackDepth;
    if (stackDepth + 1 == VM_MAX_STACK_DEPTH) {
        fprintf(stderr, "PixotronVM_stack_push: stack overflow.\n");
        exit(-1);
    }
    VirtualStackFrame *frame = PixotronVM_calloc(sizeof(VirtualStackFrame));
    frame->method = method;
    frame->pc = method->offset;
    if (stackDepth == 0) {
        frame->pre = NULL;
        context->frame = frame;
    } else {
        frame->pre = context->frame;
        context->frame = frame;
    }
    frame->sp = method->maxStackSize;
    frame->maxStackSize = method->maxStackSize;
    frame->maxLocalsSize = method->maxLocalsSize;
    frame->locals = PixotronVM_calloc(VM_VALUE_SIZE * method->maxLocalsSize);
    frame->operandStack = PixotronVM_calloc(VM_VALUE_SIZE * method->maxStackSize);
    context->stackDepth = stackDepth + 1;

    return frame;
}

extern inline void PixtronVM_PushOperand(RuntimeContext *context, const VMValue *value) {
    VirtualStackFrame *frame = context->frame;
    const int32_t sp = (int32_t) (frame->sp - 1);
    if (sp < 0) {
        context->throwException(context, "Stack underflow.");
    }
    frame->sp = sp;
    if (value == NULL) {
        return;
    }
    uint8_t *stackTop = (uint8_t *) (frame->operandStack + sp);
    memcpy(stackTop, value, VM_VALUE_SIZE);
    uint32_t t[10];
    memcpy(t, value, VM_VALUE_SIZE);
}


extern inline VMValue *PixtronVM_PopOperand(RuntimeContext *context) {
    VirtualStackFrame *frame = context->frame;
    const uint32_t sp = frame->sp;
    if (sp + 1 > frame->maxStackSize) {
        context->throwException(context, "Stack overflow.");
    }
    VMValue *value = frame->operandStack + sp;
    frame->sp = sp + 1;
    return value;
}

extern inline void PixtronVM_CreateStackFrame(RuntimeContext *context, const Method *method, const uint16_t argv,
                                              const VMValue **args) {
    const VirtualStackFrame *frame = PixtronVM_IPushStackFrame(context, method, argv);
    for (uint16_t i = 0; i < argv; i++) {
        const VMValue *arg = args[i];
        if (arg->type != (method->args + i)->type) {
            context->throwException(context, "Invalid argument.");
        }
        memcpy(frame->locals + i, arg, VM_VALUE_SIZE);
    }
}

extern inline void PixtronVM_PushStackFrame(RuntimeContext *context, const Method *method) {
    const uint16_t argv = method->argv;
    VirtualStackFrame *frame = context->frame;
    const VirtualStackFrame *newFrame = PixtronVM_IPushStackFrame(context, method, argv);
    if (argv == 0) {
        return;
    }
    const uint16_t sp = frame->sp;
    const uint16_t depth = frame->maxStackSize - sp;
    if (depth < method->argv) {
        context->throwException(context, "Stack depth too min.");
    }
    const VMValue *operandStack = frame->operandStack + sp;
    for (int i = 0; i < argv; ++i) {
        newFrame->locals[i] = operandStack[argv - i - 1];
    }
    frame->sp = sp + argv;
}

extern inline void PixtronVM_PopStackFrame(RuntimeContext *context) {
    const uint32_t depth = context->stackDepth;
    if (depth == 0) {
        fprintf(stderr, "PixotronVM_stack_pop: stack underflow.\n");
        exit(-1);
    }
    VirtualStackFrame *frame = context->frame;
    context->frame = frame->pre;
    context->stackDepth = depth - 1;
    PixotronVM_free(TO_REF(frame->locals));
    PixotronVM_free(TO_REF(frame->operandStack));
    PixotronVM_free(TO_REF(frame));
}


extern inline void PixtronVM_SetLocalTable(RuntimeContext *context, const uint16_t index, const VMValue *value) {
    g_assert(value != NULL);
    const VirtualStackFrame *frame = context->frame;
    const uint16_t maxLocalsSize = frame->maxLocalsSize;
    if (index > maxLocalsSize) {
        context->throwException(context, "Local index out of bounds.");
    }
    uint8_t *ptr = (uint8_t *) frame->locals + index;
    memcpy(ptr, value, VM_VALUE_SIZE);
}

extern inline void PixtronVM_GetLocalTable(RuntimeContext *context, const uint16_t index, VMValue *value) {
    const VirtualStackFrame *frame = context->frame;
    const uint16_t maxLocals = frame->maxLocalsSize;
    if (index > maxLocals) {
        context->throwException(context, "Local index out if bound.");
    }
    const VMValue *ptr = frame->locals + index;
    memcpy(value, ptr, VM_VALUE_SIZE);
}

extern inline void PixtronVM_MoveStackFrameSp(RuntimeContext *context, int32_t offset) {
    if (offset == 0) {
        return;
    }
    VirtualStackFrame *frame = context->frame;
    const int32_t sp = (int32_t) (frame->sp + offset);
    if (sp > frame->maxStackSize || sp < 0) {
        context->throwException(context, "Stack index out of bound.");
    }
    frame->sp = sp;
}
