//
// Created by leo on 25-5-24.
//

#include "VirtualStack.h"

#include <Config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Memory.h"

extern inline void PixtronVM_stack_push(PixtronVMPtr pixtron, const Variant *variant) {
    VirtualStackPtr stack = pixtron->stack;
    VirtualStackFramePtr frame = stack->frame;
    const size_t sp = frame->sp;
    if (sp == 0) {
        fprintf(stderr, "PixotronVM_stack_push: stack overflow\n");
        exit(1);
    }
    const uint32_t tmp = sp - 1;
    void *stackTop = frame->operandStack + tmp;
    PixtronVM_to_VMValue(variant, stackTop);
    frame->sp = tmp;
}


extern inline void PixtronVM_stack_pop(PixtronVMPtr pixtron, Variant *variant) {
    VirtualStackPtr stack = pixtron->stack;
    VirtualStackFramePtr frame = stack->frame;
    const size_t sp = frame->sp;
    if (sp + 1 > frame->maxStack) {
        fprintf(stderr, "PixotronVM_stack_pop: stack underflow.\n");
        exit(-1);
    }
    VMValue value = 0;
    const void *stackTop = frame->operandStack + sp;
    memcpy(&value, stackTop, VM_VALUE_SIZE);
    frame->sp = sp + 1;
    const DataType type = PixtronVM_typeof(value);
    memcpy(&value, &variant, TYPE_SIZE[type]);
    variant->type = type;
}

extern inline void PixtronVM_stack_frame_push(PixtronVMPtr pixtron, const size_t maxLocals, const size_t maxStack) {
    VirtualStackPtr stack = pixtron->stack;
    const size_t depth = stack->depth;
    if (depth + 1 == VM_MAX_STACK_DEPTH) {
        fprintf(stderr, "PixotronVM_stack_push: stack overflow.\n");
        exit(-1);
    }
    VirtualStackFramePtr frame = PixotronVM_calloc(sizeof(struct VirtualStackFrame));
    frame->maxStack = maxStack;
    frame->maxLocals = maxLocals;
    frame->sp = maxStack;
    if (depth == 0) {
        frame->pre = NULL;
        stack->frame = frame;
    } else {
        frame->pre = stack->frame;
        stack->frame = frame;
    }
    frame->operandStack = PixotronVM_calloc(VM_VALUE_SIZE * maxStack);
    frame->localVarTable = PixotronVM_calloc(VM_VALUE_SIZE * maxLocals);
    stack->depth = depth + 1;
}

extern inline void PixtronVM_stack_frame_pop(PixtronVMPtr pixtron) {
    VirtualStackPtr stack = pixtron->stack;
    const size_t depth = stack->depth;
    if (depth == 0) {
        fprintf(stderr, "PixotronVM_stack_pop: stack underflow.\n");
        exit(-1);
    }
    VirtualStackFramePtr frame = stack->frame;
    stack->frame = frame->pre;
    stack->depth = depth - 1;
    PixotronVM_free(TO_REF(frame->operandStack));
    PixotronVM_free(TO_REF(frame->localVarTable));
    PixotronVM_free(TO_REF(frame));
}
