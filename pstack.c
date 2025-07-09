#include "pstack.h"

#include <config.h>
#include <stdio.h>

#define GET_OPERAND_STACK(frame) ((VMValue *)(frame+1))
#define GET_LOCALS(frame) (((VMValue *) (frame + 1)) + frame->max_stacks)

static inline VirtualStackFrame *pvm_ipush_stack_frame(RuntimeContext *context, const Method *method,
                                                       const uint16_t argc) {
    const uint32_t sp = context->sp;
    const uint16_t max_locals = method->max_locals;
    const uint16_t max_stacks = method->max_stacks;
    if (max_locals < argc) {
        context->throw_exception(context, "Method param count mistake.");
    }
    const uint32_t vstack_size = sizeof(VirtualStackFrame) + ((max_locals + max_stacks) * VM_VALUE_SIZE);
    if (vstack_size + sp > VM_STACK_SIZE) {
        context->throw_exception(context, "PixotronVM_stack_push: stack overflow.\n");
    }
    const Klass *klass = method->klass;
    VirtualStackFrame *frame = (VirtualStackFrame *) (context->stack + sp);
    if (frame->method != method) {
        frame->method = method;
        frame->max_locals = max_locals;
        frame->max_stacks = max_stacks;
        frame->bytecode = klass->bytecode;
    }
    frame->sp = max_stacks;
    frame->pc = method->offset;
    frame->pre = context->frame;
    context->sp = sp + vstack_size;
    context->frame = frame;
    context->stack_depth++;
    return frame;
}

extern inline void pvm_push_operand(const RuntimeContext *context, const VMValue *value) {
    VirtualStackFrame *frame = context->frame;
    const int32_t sp = (int32_t) (frame->sp - 1);
#if VM_DEBUG_ENABLE
    if (sp < 0) {
        context->throw_exception(context, "Stack pointer out of bounds.");
    }
#endif
    frame->sp = sp;
    uint8_t *stack_top = (uint8_t *) (GET_OPERAND_STACK(frame) + sp);
    memcpy(stack_top, value, VM_VALUE_SIZE);
}


extern inline VMValue *pvm_pop_operand(const RuntimeContext *context) {
    VirtualStackFrame *frame = context->frame;
    const uint32_t sp = frame->sp;
    VMValue *value = GET_OPERAND_STACK(frame) + sp;
    frame->sp = sp + 1;
    return value;
}

extern inline void pvm_create_stack_frame(RuntimeContext *context, const Method *method, const uint16_t argc,
                                          const VMValue **argv) {
    const VirtualStackFrame *frame = pvm_ipush_stack_frame(context, method, argc);
    for (uint16_t i = 0; i < argc; i++) {
        const VMValue *arg = argv[i];
        if (arg->type != (method->argv + i)->type) {
            context->throw_exception(context, "Invalid argument.");
        }
        VMValue *locals = GET_LOCALS(frame);
        memcpy(locals + i, arg, VM_VALUE_SIZE);
    }
}

extern inline void pvm_push_stack_frame(RuntimeContext *context, const Method *method) {
    const uint16_t argc = method->argc;
    VirtualStackFrame *frame = context->frame;
    const VirtualStackFrame *new_frame = pvm_ipush_stack_frame(context, method, argc);
    if (argc == 0) {
        return;
    }
    const uint32_t sp = frame->sp;
    const VMValue *operand_stack = GET_OPERAND_STACK(frame) + sp + argc - 1;
    VMValue *locals = GET_LOCALS(new_frame);
    if (argc == 1) {
        memcpy(locals, operand_stack, VM_VALUE_SIZE);
    } else {
        for (int i = 0; i < argc; ++i) {
            memcpy(locals + i, operand_stack--, VM_VALUE_SIZE);
        }
    }
    frame->sp = sp + argc;
}

extern inline void pvm_pop_stack_frame(RuntimeContext *context) {
    const uint32_t sp = context->sp;
    if (sp == 0) {
        return;
    }
    const VirtualStackFrame *frame = context->frame;
    context->frame = frame->pre;
    if (context->frame == NULL) {
        context->sp = 0;
    } else {
        context->sp = ((uint8_t *) frame) - context->stack;
    }
}


extern inline void pvm_set_local_value(const RuntimeContext *context, const uint16_t index, const VMValue *value) {
    const VirtualStackFrame *frame = context->frame;
    VMValue *locals = GET_LOCALS(frame) + index;
    memcpy(locals, value, VM_VALUE_SIZE);
}

extern inline void pvm_copy_local_value(const RuntimeContext *context, const uint16_t index, VMValue *value) {
    const VirtualStackFrame *frame = context->frame;
    const VMValue *ptr = GET_LOCALS(frame) + index;
    memcpy(value, ptr, VM_VALUE_SIZE);
}

extern inline VMValue *pvm_get_local_value(const RuntimeContext *context, uint16_t index) {
    const VirtualStackFrame *frame = context->frame;
    VMValue *value = GET_LOCALS(frame) + index;
    return value;
}


extern inline VMValue *pvm_get_operand(const RuntimeContext *context) {
    const VirtualStackFrame *frame = context->frame;
    const uint32_t sp = frame->sp;
    VMValue *operand_stack = GET_OPERAND_STACK(frame);
    return operand_stack + sp;
}

extern inline VMValue *pvm_next_operand(const RuntimeContext *context) {
    VirtualStackFrame *frame = context->frame;
    const int32_t sp = (int32_t) frame->sp - 1;
#if VM_DEBUG_ENABLE
    if (sp < 0) {
        context->throw_exception(context, "Stack pointer out of bounds.");
    }
#endif
    VMValue *operand_stack = GET_OPERAND_STACK(frame);
    frame->sp = sp;
    return operand_stack + sp;
}
