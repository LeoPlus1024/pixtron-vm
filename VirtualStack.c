#include "VirtualStack.h"

#include <assert.h>
#include <Config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Memory.h"

extern inline void PixtronVM_PushOperand(RuntimeContext *context, const Variant *variant) {
    VirtualStackFrame *frame = context->frame;
    const size_t sp = frame->sp;
    if (sp == 0) {
        context->throwException(context, "Stack underflow.");
    }
    const guint tmp = sp - 1;
    guint8 *stackTop = (guint8 *) (frame->operandStack + tmp);
    PixtronVM_ConvertValueToBuffer(variant, stackTop);
    frame->sp = tmp;
}


extern inline void PixtronVM_PopOperand(RuntimeContext *context, Variant *variant) {
    VirtualStackFrame *frame = context->frame;
    const Method *method = frame->method;
    const size_t sp = frame->sp;
    if (sp + 1 > method->maxStacks) {
        fprintf(stderr, "PixotronVM_stack_pop: stack underflow.\n");
        exit(-1);
    }
    VMValue value = 0;
    const void *stackTop = frame->operandStack + sp;
    memcpy(&value, stackTop, VM_VALUE_SIZE);
    frame->sp = sp + 1;
    PixtronVM_ConvertValueToVariant(value, variant);
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
    frame->operandStack = PixotronVM_calloc(VM_VALUE_SIZE * method->maxStacks);
    frame->localVarTable = PixotronVM_calloc(VM_VALUE_SIZE * method->maxLocals);
    stack->depth = depth + 1;
}

extern inline void PixtronVM_PopStackFrame(RuntimeContext *context) {
    VirtualStack *stack = context->stack;
    const size_t depth = stack->depth;
    if (depth == 0) {
        fprintf(stderr, "PixotronVM_stack_pop: stack underflow.\n");
        exit(-1);
    }
    VirtualStackFramePtr frame = context->frame;
    context->frame = frame->pre;
    stack->depth = depth - 1;
    PixotronVM_free(TO_REF(frame->operandStack));
    PixotronVM_free(TO_REF(frame->localVarTable));
    PixotronVM_free(TO_REF(frame));
}


extern inline void PixtronVM_SetLocalTable(RuntimeContext *context, uint16_t index, const Variant *variant) {
    g_assert(variant != NULL);
    const VirtualStackFrame *frame = context->frame;
    g_assert(frame!=NULL);
    const Method *method = frame->method;
    g_assert(frame != NULL);
    const uint16_t maxLocals = method->maxLocals;
    g_assert(index < maxLocals);
    const VMValue tmp = frame->localVarTable[index];
    const Type t0 = PixtronVM_GetValueType(tmp);
    g_assert((t0 == NIL || t0==variant->type)&&"Local variable table type mistake.");
    guint8 *ptr = (guint8 *) frame->localVarTable + index;
    PixtronVM_ConvertValueToBuffer(variant, ptr);
}

extern inline void PixtronVM_GetLocalTable(RuntimeContext *context, uint16_t index, Variant *variant) {
    g_assert(variant!=NULL);
    const VirtualStackFrame *frame = context->frame;
    g_assert(frame != NULL);
    const Method *method = frame->method;
    const uint16_t maxLocals = method->maxLocals;
    g_assert(index < maxLocals);
    const VMValue tmp = frame->localVarTable[index];
    PixtronVM_ConvertValueToVariant(tmp, variant);
}
