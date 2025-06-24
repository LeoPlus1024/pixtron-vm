#include "stack.h"

#include <assert.h>
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include "memory.h"

static inline VirtualStackFrame *pvm_ipush_stack_frame(RuntimeContext *context, const Method *method,
                                                       const uint16_t argv) {
    if (argv != method->argv) {
        context->throwException(context, "Invalid number of arguments expect argv is %d but is %d", method->argv, argv);
    }
    const uint32_t stackDepth = context->stackDepth;
    if (stackDepth + 1 == VM_MAX_STACK_DEPTH) {
        fprintf(stderr, "PixotronVM_stack_push: stack overflow.\n");
        exit(-1);
    }
    VirtualStackFrame *frame = pvm_mem_calloc(sizeof(VirtualStackFrame));
    frame->method = method;
    frame->pc = method->offset;
    if (stackDepth == 0) {
        frame->pre = NULL;
        context->frame = frame;
    } else {
        frame->pre = context->frame;
        context->frame = frame;
    }
    frame->sp = method->max_stacks;
    frame->max_stacks = method->max_stacks;
    frame->max_locals = method->max_locals;
    frame->locals = pvm_mem_calloc(VM_VALUE_SIZE * method->max_locals);
    frame->operand_stack = pvm_mem_calloc(VM_VALUE_SIZE * method->max_stacks);
    context->stackDepth = stackDepth + 1;

    return frame;
}

extern inline void pvm_stack_fram_dispose(VirtualStackFrame **frame) {
    if (frame == NULL || *frame == NULL) {
        return;
    }
    VirtualStackFrame *self = *frame;
    pvm_mem_free(TO_REF(self->locals));
    pvm_mem_free(TO_REF(self->operand_stack));
    pvm_mem_free(CAST_REF(frame));
}

extern inline void pvm_push_operand(RuntimeContext *context, const VMValue *value) {
    g_assert(value!=NULL);
    VirtualStackFrame *frame = context->frame;
    const int32_t sp = (int32_t) (frame->sp - 1);
    if (sp < 0) {
        context->throwException(context, "Stack underflow.");
    }
    frame->sp = sp;
    uint8_t *stack_top = (uint8_t *) (frame->operand_stack + sp);
    memcpy(stack_top, value, VM_VALUE_SIZE);
}


extern inline VMValue *pvm_pop_operand(RuntimeContext *context) {
    VirtualStackFrame *frame = context->frame;
    const uint32_t sp = frame->sp;
    if (sp + 1 > frame->max_stacks) {
        context->throwException(context, "Stack overflow.");
    }
    VMValue *value = frame->operand_stack + sp;
    frame->sp = sp + 1;
    return value;
}

extern inline void pvm_create_stack_frame(RuntimeContext *context, const Method *method, const uint16_t argv,
                                          const VMValue **args) {
    const VirtualStackFrame *frame = pvm_ipush_stack_frame(context, method, argv);
    for (uint16_t i = 0; i < argv; i++) {
        const VMValue *arg = args[i];
        if (arg->type != (method->args + i)->type) {
            context->throwException(context, "Invalid argument.");
        }
        memcpy(frame->locals + i, arg, VM_VALUE_SIZE);
    }
}

extern inline void pvm_push_stack_frame(RuntimeContext *context, const Method *method) {
    const uint16_t argv = method->argv;
    VirtualStackFrame *frame = context->frame;
    const VirtualStackFrame *new_frame = pvm_ipush_stack_frame(context, method, argv);
    if (argv == 0) {
        return;
    }
    const uint16_t sp = frame->sp;
    const uint16_t depth = frame->max_stacks - sp;
    if (depth < method->argv) {
        context->throwException(context, "Stack depth too min.");
    }
    const VMValue *operand_stack = frame->operand_stack + sp;
    for (int i = 0; i < argv; ++i) {
        new_frame->locals[i] = operand_stack[argv - i - 1];
    }
    frame->sp = sp + argv;
}

extern inline void pvm_pop_stack_frame(RuntimeContext *context) {
    const uint32_t depth = context->stackDepth;
    if (depth == 0) {
        g_warning("Stack frame underflow.");
        exit(-1);
    }
    const VirtualStackFrame *frame = context->frame;
    context->frame = frame->pre;
    context->stackDepth = depth - 1;
}


extern inline void pvm_set_local_value(RuntimeContext *context, const uint16_t index, const VMValue *value) {
    g_assert(value != NULL);
    const VirtualStackFrame *frame = context->frame;
    const uint16_t max_locals = frame->max_locals;
    if (index > max_locals) {
        context->throwException(context, "Local index out of bounds.");
    }
    uint8_t *ptr = (uint8_t *) (frame->locals + index);
    memcpy(ptr, value, VM_VALUE_SIZE);
}

extern inline void pvm_get_local_value(RuntimeContext *context, const uint16_t index, VMValue *value) {
    const VirtualStackFrame *frame = context->frame;
    const uint16_t max_locals = frame->max_locals;
    if (index > max_locals) {
        context->throwException(context, "Local index out if bound.");
    }
    const VMValue *ptr = frame->locals + index;
    memcpy(value, ptr, VM_VALUE_SIZE);
}

extern inline void pvm_move_stack_pointer(RuntimeContext *context, const int32_t offset) {
    if (offset == 0) {
        return;
    }
    VirtualStackFrame *frame = context->frame;
    const int32_t sp = (int32_t) (frame->sp + offset);
    if (sp > frame->max_stacks || sp < 0) {
        context->throwException(context, "Stack index out of bound.");
    }
    frame->sp = sp;
}
