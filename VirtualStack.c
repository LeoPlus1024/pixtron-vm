#include "VirtualStack.h"

#include <assert.h>
#include <Config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Memory.h"

extern inline void PixtronVM_PushOperand(RuntimeContext *context, const VMValue *value) {
    VirtualStackFrame *frame = context->frame;
    const size_t sp = frame->sp;
    if (sp == 0) {
        context->throwException(context, "Stack underflow.");
    }
    const guint tmp = sp - 1;
    uint8_t *stackTop = (uint8_t *) (frame->operandStack + tmp);
    memcpy(stackTop, value, VM_VALUE_SIZE);
    frame->sp = tmp;
}


extern inline VMValue *PixtronVM_PopOperand(RuntimeContext *context) {
    VirtualStackFrame *frame = context->frame;
    const size_t sp = frame->sp;
    if (sp + 1 == frame->maxStackSize) {
        context->throwException(context, "Stack overflow.");
    }
    VMValue *value = frame->operandStack;
    frame->sp = sp + 1;
    return value;
}

extern inline void PixtronVM_PushStackFrame(RuntimeContext *context, const Method *method) {
    VirtualStack *stack = context->stack;
    const size_t depth = stack->depth;
    if (depth + 1 == VM_MAX_STACK_DEPTH) {
        fprintf(stderr, "PixotronVM_stack_push: stack overflow.\n");
        exit(-1);
    }
    VirtualStackFrame *frame = PixotronVM_calloc(sizeof(VirtualStackFrame));
    frame->method = method;
    frame->pc = method->offset;
    if (depth == 0) {
        frame->pre = NULL;
        context->frame = frame;
    } else {
        frame->pre = context->frame;
        context->frame = frame;
    }
    frame->sp = method->maxStackSize - 1;
    frame->maxStackSize = method->maxStackSize;
    frame->maxLocalsSize = method->maxLocalsSize;
    frame->locals = PixotronVM_calloc(VM_VALUE_SIZE * method->maxLocalsSize);
    frame->operandStack = PixotronVM_calloc(VM_VALUE_SIZE * method->maxStackSize);
    stack->depth = depth + 1;
}

extern inline void PixtronVM_PopStackFrame(RuntimeContext *context) {
    VirtualStack *stack = context->stack;
    const size_t depth = stack->depth;
    if (depth == 0) {
        fprintf(stderr, "PixotronVM_stack_pop: stack underflow.\n");
        exit(-1);
    }
    VirtualStackFrame *frame = context->frame;
    context->frame = frame->pre;
    stack->depth = depth - 1;
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
    guint8 *ptr = (guint8 *) frame->locals + index;
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
