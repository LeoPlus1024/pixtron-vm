#include "VirtualStack.h"

#include <assert.h>
#include <Config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Memory.h"

extern inline void PixtronVM_stack_push(PixtronVMPtr vm, const Variant *variant) {
    VirtualStackPtr stack = vm->stack;
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


extern inline void PixtronVM_stack_pop(PixtronVMPtr vm, Variant *variant) {
    VirtualStackPtr stack = vm->stack;
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
    const Type type = PixtronVM_typeof(value);
    if (type == TYPE_DOUBLE || type == TYPE_LONG) {
        memcpy(&(variant->value), &value, TYPE_SIZE[type]);
    }
    // 对于 byte、short类型直接扩容到int类型
    else {
        variant->value.i = (int32_t) value;
    }
    variant->type = type;
}

extern inline void PixtronVM_stack_frame_push(PixtronVMPtr vm, const uint16_t maxLocals, const uint16_t maxStack) {
    VirtualStackPtr stack = vm->stack;
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

extern inline void PixtronVM_stack_frame_pop(PixtronVMPtr vm) {
    VirtualStackPtr stack = vm->stack;
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


extern inline void PixtronVM_stack_ltable_set(PixtronVMPtr vm, uint16_t index, const VMValue *value) {
    assert(value != NULL);
    VirtualStackFramePtr frame = vm->stack->frame;
    assert(frame != NULL);
    const uint16_t maxLocals = frame->maxLocals;
    assert(index < maxLocals);
    const VMValue tmp = frame->localVarTable[index];
    const Type t0 = PixtronVM_typeof(tmp);
    if (t0 == NIL) {
        frame->localVarTable[index] = tmp;
    } else {
        const Type t1 = PixtronVM_typeof(tmp);
        assert(t0==t1);
        frame->localVarTable[index] = *value;
    }
}
