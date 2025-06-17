#include "Engine.h"

#include <assert.h>
#include <Config.h>
#include <stdbool.h>

#include "ByteCodeReader.h"
#include "Memory.h"
#include "VirtualStack.h"


static void PixtronVM_GetOpsData(const RuntimeContext *context, Variant *variant) {
    const guint8 subOps = PixtronVM_ReadByteCodeU8(context);
    const Type type = OPS_DATA_TYPE(subOps);
    const DataSource source = OPS_DATA_SOURCE(subOps);
    variant->type = type;
    // Immediate
    if (source == IMMEDIATE) {
        PixtronVM_ReadByteCodeImm(context, variant);
    }
    // Local
    else {
        const guint16 index = PixtronVM_ReadByteCodeU16(context);
        const bool local = source == LOCAL_VAR;
        if (local) {
            PixtronVM_GetLocalTable(context, index, variant);
        } else {
            //
        }
    }
}

static void PixotronVM_AddSub(RuntimeContext *context, Opcode opcode) {
    Variant top;
    Variant next;

    PixtronVM_StackPop(context, &top);
    PixtronVM_StackPop(context, &next);

    assert(VM_TYPE_NUMBER(top.type) && "Top operand must be number.");
    assert(VM_TYPE_NUMBER(next.type) && "Next operand must be number.");

    if (opcode == SBC) {
        PixtronVM_negative(&top);
    }

    // 类型相同时直接运算
    if (top.type == next.type) {
        switch (top.type) {
#define HANDLE_CASE(type, op) \
        case type: { \
            next.value.op += top.value.op; \
            break; \
        }
            HANDLE_CASE(TYPE_BYTE, b)
            HANDLE_CASE(TYPE_SHORT, s)
            HANDLE_CASE(TYPE_INT, i)
            case TYPE_LONG:
                next.value.l = CLONG_TO_VLONG(next.value.l + top.value.l);
                break;
            HANDLE_CASE(TYPE_DOUBLE, d)
#undef HANDLE_CASE
            default:
                assert(false && "Unhandled numeric type");
        }
    }
    // Double自动扩展
    else if (VM_TYPE_DOUBLE(top.type) || VM_TYPE_DOUBLE(next.type)) {
        PixtronVM_ConvertToDoubleValue(&top);
        PixtronVM_ConvertToDoubleValue(&next);
        const double sum = next.value.d + top.value.d;
        next.value.d = sum;
    }
    // Long自动扩展
    else if (VM_TYPE_LONG(top.type) || VM_TYPE_LONG(next.type)) {
        PixtronVM_ConvertToLongValue(&top);
        PixtronVM_ConvertToLongValue(&next);
        next.value.l = CLONG_TO_VLONG(next.value.l + top.value.l);
    }
    // Byte、Short、Int
    else {
        next.value.i = next.value.i + top.value.i;
        if (VM_TYPE_BYTE(top.type) || VM_TYPE_BYTE(next.type)) {
            next.type = TYPE_BYTE;
        } else if (VM_TYPE_SHORT(top.type) || VM_TYPE_SHORT(next.type)) {
            next.type = TYPE_SHORT;
        } else {
            next.type = TYPE_INT;
        }
    }
    PixtronVM_StackPush(context, &next);
}

static void PixtronVM_CheckCon(RuntimeContext *context, Opcode opcode) {
    uint16_t offset;
    if (opcode == GOTO) {
        offset = PixtronVM_ReadByteCodeU16(context);
    } else {
        const Variant variant;
        PixtronVM_StackPop(context, &variant);
        const bool ifeq = opcode == IFEQ;
        assert(VM_TYPE_BOOL(variant.type)&&"Ifeq or Ifnq only support boolean.");
        if ((ifeq && VM_FALSE(variant)) || !ifeq && VM_TRUE(variant)) {
            offset = PixtronVM_ReadByteCodeU16(context);
        } else {
            offset = 0;
        }
    }
    VirtualStackFrame *frame = context->frame;
    frame->pc = frame->pc + offset;
}

static void PixtronVM_ICmp(RuntimeContext *context, Opcode opcode) {
    Variant top;
    Variant next;
    PixtronVM_StackPop(context, &top);
    PixtronVM_StackPop(context, &next);
    next.value.i = SIGN_CMP(next.value.i, top.value.i);
    PixtronVM_StackPush(context, &next);
}

//
// extern inline void PixtronVM_exec_cmp(PixtronVMPtr vm, Opcode opcode) {
//     Variant top;
//     Variant next;
//     PixtronVM_stack_pop(vm, &top);
//     PixtronVM_stack_pop(vm, &next);
//     switch (opcode) {
//         case ICMP:
//             next.value.i = SIGN_CMP(next.value.i, top.value.i);
//             break;
//         case DCMP:
//             next.value.i = SIGN_CMP(next.value.d, top.value.d);
//             break;
//         case LCMP:
//             next.value.i = SIGN_CMP(next.value.l, top.value.l);
//             break;
//         default:
//             assert(false && "Unhandled cmp opcode.");
//     }
//     PixtronVM_stack_push(vm, &next);
// }
//
// extern inline void PixtronVM_exec_conv(PixtronVMPtr vm) {
//     const Type type = PixtronVM_code_segment_u8(vm);
//     Variant variant;
//     PixtronVM_stack_pop(vm, &variant);
//     const Type src = variant.type;
//     if (src == type) {
//         return;
//     }
//     assert(VM_TYPE_NUMBER(src)&&"Only support cast conv basic type.");
//     assert(VM_TYPE_NUMBER(type));
//     if (type == TYPE_DOUBLE) {
//         PixtronVM_ConvertToDoubleValue(&variant);
//     } else if (type == TYPE_LONG) {
//         PixtronVM_ConvertToLongValue(&variant);
//     } else {
//         variant.type = type;
//     }
//     PixtronVM_stack_push(vm, &variant);
// }
//
extern inline void PixtronVM_Store(RuntimeContext *context) {
    const Variant value;
    PixtronVM_StackPop(context, &value);
    const uint8_t subOps = PixtronVM_ReadByteCodeU8(context);
    const DataSource s = OPS_DATA_SOURCE(subOps);
    // Global variable
    if (s == GLOBAL_VAR) {
    }
    // Local variable
    else if (s == LOCAL_VAR) {
        const uint16_t index = PixtronVM_ReadByteCodeU16(context);
        PixtronVM_SetLocalTable(context, index, &value);
    }
}


static void PixtronVM_Load(RuntimeContext *context) {
    Variant value;
    PixtronVM_GetOpsData(context, &value);
    PixtronVM_StackPush(context, &value);
}

extern Variant *PixtronVM_CallMethod(const Method *method) {
    RuntimeContext *context = PixotronVM_calloc(sizeof(RuntimeContext));
    context->vm = method->klass->vm;
    VirtualStack *stack = PixotronVM_calloc(sizeof(VirtualStack));
    stack->maxDepth = VM_MAX_STACK_DEPTH;
    stack->depth = 0;
    context->stack = stack;
    PixtronVM_StackFramePush(context, method);

    while (context->frame != NULL) {
        const Opcode ops = PixtronVM_ReadByteCodeU8(context);

        switch (ops) {
            case LOAD:
                PixtronVM_Load(context);
                break;
            case STORE:
                PixtronVM_Store(context);
                break;
            case ADD:
            case SBC:
                PixotronVM_AddSub(context, ops);
                break;
            case GOTO:
            case IFEQ:
            case IFNE:
                PixtronVM_CheckCon(context, ops);
                break;
            case ICMP:
            case DCMP:
            case LCMP:
                PixtronVM_ICmp(context, ops);
                break;
                //         case CONV:
                //             PixtronVM_exec_conv(vm);
                //             break;
                //         case CALL:
                //             break;
                //         default:
                //
                //
                //
        }
    }
}
