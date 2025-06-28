#include "stack.h"

#include <config.h>
#include <stdio.h>

#define GET_OPERAND_STACK(frame) ((VMValue *)(frame+1))
#define GET_LOCALS(frame) (((VMValue *) (frame + 1)) + frame->max_stacks)

static inline VirtualStackFrame *pvm_ipush_stack_frame(RuntimeContext *context, const Method *method,
                                                       const uint16_t argv) {
    const uint32_t sp = context->sp;
    const uint16_t max_locals = method->max_locals;
    const uint16_t max_stacks = method->max_stacks;
    if (max_locals < argv) {
        context->throw_exception(context, "Method param count mistake.");
    }
    const uint32_t vstack_size = sizeof(VirtualStackFrame) + ((max_locals + max_stacks) * VM_VALUE_SIZE);
    if (vstack_size + sp > VM_STACK_SIZE) {
        context->throw_exception(context, "PixotronVM_stack_push: stack overflow.\n");
    }
    VirtualStackFrame *frame = (VirtualStackFrame *) (context->stack + sp);
    frame->sp = max_stacks;
    frame->method = method;
    frame->pc = method->offset;
    frame->pre = context->frame;
    frame->max_locals = max_locals;
    frame->max_stacks = max_stacks;
    context->sp = sp + vstack_size;
    context->frame = frame;
    context->stack_depth++;
    return frame;
}

extern inline void pvm_push_operand(RuntimeContext *context, const VMValue *value) {
#if VM_DEBUG_ENABLE
    g_assert(value!=NULL);
#endif
    VirtualStackFrame *frame = context->frame;
    const int32_t sp = (int32_t) (frame->sp - 1);
    if (sp < 0) {
        context->throw_exception(context, "Stack underflow.");
    }
    frame->sp = sp;
    uint8_t *stack_top = (uint8_t *) (GET_OPERAND_STACK(frame) + sp);
    memcpy(stack_top, value, VM_VALUE_SIZE);
}


extern inline VMValue *pvm_pop_operand(RuntimeContext *context) {
    VirtualStackFrame *frame = context->frame;
    const uint32_t sp = frame->sp;
    if (sp + 1 > frame->max_stacks) {
        context->throw_exception(context, "Stack overflow.");
    }
    VMValue *value = GET_OPERAND_STACK(frame) + sp;
    frame->sp = sp + 1;
    return value;
}

extern inline void pvm_create_stack_frame(RuntimeContext *context, const Method *method, const uint16_t argv,
                                          const VMValue **args) {
    const VirtualStackFrame *frame = pvm_ipush_stack_frame(context, method, argv);
    for (uint16_t i = 0; i < argv; i++) {
        const VMValue *arg = args[i];
        if (arg->type != (method->args + i)->type) {
            context->throw_exception(context, "Invalid argument.");
        }
        VMValue *locals = GET_LOCALS(frame);
        memcpy(locals + i, arg, VM_VALUE_SIZE);
    }
}

extern inline void pvm_push_stack_frame(RuntimeContext *context, const Method *method) {
    const uint16_t argv = method->argv;
    VirtualStackFrame *frame = context->frame;
    const VirtualStackFrame *new_frame = pvm_ipush_stack_frame(context, method, argv);
    if (argv == 0) {
        return;
    }
    const uint32_t sp = frame->sp;
    const uint32_t depth = frame->max_stacks - sp;
    if (depth < argv) {
        context->throw_exception(context, "Stack depth too min.");
    }
    if (method->max_locals < argv) {
        context->throw_exception(context, "Method local variable index out of bounds: max %d, given %d",
                                 method->max_locals, argv);
    }
    const VMValue *operand_stack = GET_OPERAND_STACK(frame) + sp;
    VMValue *locals = GET_LOCALS(new_frame);
    for (int i = 0; i < argv; ++i) {
        locals[i] = operand_stack[argv - i - 1];
    }
    frame->sp = sp + argv;
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


extern inline void pvm_set_local_value(RuntimeContext *context, const uint16_t index, const VMValue *value) {
#if VM_DEBUG_ENABLE
    g_assert(value != NULL);
#endif
    const VirtualStackFrame *frame = context->frame;
    const uint16_t max_locals = frame->max_locals;
    if (index > max_locals) {
        context->throw_exception(context, "Local index out of bounds.");
    }
    VMValue *locals = GET_LOCALS(frame);
    uint8_t *ptr = (uint8_t *) (locals + index);
    memcpy(ptr, value, VM_VALUE_SIZE);
}

extern inline void pvm_get_local_value(RuntimeContext *context, const uint16_t index, VMValue *value) {
    const VirtualStackFrame *frame = context->frame;
    const uint16_t max_locals = frame->max_locals;
    if (index > max_locals) {
        context->throw_exception(context, "Local index out if bound.");
    }
    const VMValue *ptr = GET_LOCALS(frame) + index;
    memcpy(value, ptr, VM_VALUE_SIZE);
}


extern inline VMValue *pvm_get_operand(RuntimeContext *context) {
    const VirtualStackFrame *frame = context->frame;
    const uint32_t sp = frame->sp;
    if (sp >= frame->max_stacks) {
        context->throw_exception(context, "Stack index out of bound.");
    }
    VMValue *operand_stack = GET_OPERAND_STACK(frame);
    return operand_stack + sp;
}
