#include "VirtualStack.h"

#include <assert.h>
#include <Config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Memory.h"

extern inline void PixtronVM_StackPush(RuntimeContext *context, const Variant *variant) {
    VirtualStackFramePtr frame = context->frame;
    const size_t sp = frame->sp;
    if (sp == 0) {
        fprintf(stderr, "PixotronVM_stack_push: stack overflow\n");
        exit(1);
    }
    const guint tmp = sp - 1;
    const void *stackTop = frame->operandStack + tmp;
    PixtronVM_ConvertValueToBuffer(variant, stackTop);
    frame->sp = tmp;
}


extern inline void PixtronVM_StackPop(RuntimeContext *context, const Variant *variant) {
    VirtualStackFramePtr frame = context->frame;
    const size_t sp = frame->sp;
    if (sp + 1 > frame->maxStack) {
        fprintf(stderr, "PixotronVM_stack_pop: stack underflow.\n");
        exit(-1);
    }
    VMValue value = 0;
    const void *stackTop = frame->operandStack + sp;
    memcpy(&value, stackTop, VM_VALUE_SIZE);
    frame->sp = sp + 1;
    PixtronVM_ConvertValueToVariant(value, variant);
}

extern inline void PixtronVM_StackFramePush(RuntimeContext *context, const Method *method) {
    VirtualStack *stack = context->stack;
    const size_t depth = stack->depth;
    if (depth + 1 == VM_MAX_STACK_DEPTH) {
        fprintf(stderr, "PixotronVM_stack_push: stack overflow.\n");
        exit(-1);
    }
    VirtualStackFrame *frame = PixotronVM_calloc(sizeof(VirtualStackFrame));
    frame->method = method;
    frame->maxStack = method->stacks;
    frame->maxLocals = method->locals;
    frame->pc = method->offset;
    if (depth == 0) {
        frame->pre = NULL;
        context->frame = frame;
    } else {
        frame->pre = context->frame;
        context->frame = frame;
    }
    frame->operandStack = PixotronVM_calloc(VM_VALUE_SIZE * frame->maxStack);
    frame->localVarTable = PixotronVM_calloc(VM_VALUE_SIZE * frame->maxLocals);
    stack->depth = depth + 1;
}

extern inline void PixtronVM_StackFramePop(RuntimeContext *context) {
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
    assert(variant != NULL);
    const VirtualStackFramePtr frame = context->frame;
    assert(frame != NULL);
    const uint16_t maxLocals = frame->maxLocals;
    assert(index < maxLocals);
    const VMValue tmp = frame->localVarTable[index];
    const Type t0 = PixtronVM_GetValueType(tmp);
    assert((t0 == NIL || t0==variant->type)&&"Local variable table type mistake.");
    const uint8_t *ptr = (uint8_t *) frame->localVarTable + index;
    PixtronVM_ConvertValueToBuffer(variant, ptr);
}

extern inline void PixtronVM_GetLocalTable(RuntimeContext *context, uint16_t index, Variant *variant) {
    assert(variant!=NULL);
    const VirtualStackFramePtr frame = context->frame;
    assert(frame != NULL);
    const uint16_t maxLocals = frame->maxLocals;
    assert(index < maxLocals);
    const VMValue tmp = frame->localVarTable[index];
    PixtronVM_ConvertValueToVariant(tmp, variant);
}
