#include "Engine.h"

#include <assert.h>
#include <Config.h>
#include <stdbool.h>

#include "ByteCodeReader.h"
#include "Klass.h"
#include "Memory.h"
#include "VirtualStack.h"


static void PixtronVM_GetOpsData(RuntimeContext *context, Variant *variant) {
    const guint8 subOps = PixtronVM_ReadByteCodeU8(context);
    const Type type = OPS_DATA_TYPE(subOps);
    const DataSource source = OPS_DATA_SOURCE(subOps);
    variant->type = type;
    // Immediate
    if (source == IMMEDIATE) {
        PixtronVM_ReadByteCodeImm(context, variant);
    } else {
        const guint16 index = PixtronVM_ReadByteCodeU16(context);
        const bool local = source == LOCAL_VAR;
        // Local
        if (local) {
            PixtronVM_GetLocalTable(context, index, variant);
        }
        // Klass field
        else {
            PixtronVM_GetKlassFileValue(context, index, variant);
        }
    }
}

static void PixotronVM_AddSub(RuntimeContext *context, Opcode opcode) {
    Variant top;
    Variant next;

    PixtronVM_PopOperand(context, &top);
    PixtronVM_PopOperand(context, &next);

    assert(VM_TYPE_NUMBER(top.type) && "Top operand must be number.");
    assert(VM_TYPE_NUMBER(next.type) && "Next operand must be number.");

    if (opcode == SBC) {
        PixtronVM_negative(&top);
    }

    // Same type direct operator
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
    // Auto extension to double
    else if (VM_TYPE_DOUBLE(top.type) || VM_TYPE_DOUBLE(next.type)) {
        PixtronVM_ConvertToDoubleValue(&top);
        PixtronVM_ConvertToDoubleValue(&next);
        const double sum = next.value.d + top.value.d;
        next.value.d = sum;
    }
    // Auto extension to long
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
    PixtronVM_PushOperand(context, &next);
}

static void PixtronVM_CheckCon(RuntimeContext *context, const Opcode opcode) {
    const gint16 offset = (gint16) PixtronVM_ReadByteCodeU16(context);
    if (opcode != GOTO) {
        Variant variant;
        PixtronVM_PopOperand(context, &variant);
        const bool ifeq = opcode == IFEQ;
        assert(VM_TYPE_BOOL(variant.type)&&"Ifeq or Ifnq only support boolean.");
        if (!((ifeq && VM_FALSE(variant)) || !ifeq && VM_TRUE(variant))) {
            return;
        }
    }
    VirtualStackFrame *frame = context->frame;
    // Offset contain current opcode
    frame->pc = frame->pc + offset - 2;
}


extern inline void PixtronVM_Cmp(RuntimeContext *context, Opcode opcode) {
    Variant top;
    Variant next;
    PixtronVM_PopOperand(context, &top);
    PixtronVM_PopOperand(context, &next);
    switch (opcode) {
        case ICMP:
            next.value.i = SIGN_CMP(next.value.i, top.value.i);
            break;
        case DCMP:
            next.value.i = SIGN_CMP(next.value.d, top.value.d);
            break;
        case LCMP:
            next.value.i = SIGN_CMP(next.value.l, top.value.l);
            break;
        default:
            assert(false && "Unhandled cmp opcode.");
    }
    PixtronVM_PushOperand(context, &next);
}

static void PixtronVM_CONV(RuntimeContext *context) {
    const Type type = PixtronVM_ReadByteCodeU16(context);
    Variant variant;
    PixtronVM_PopOperand(context, &variant);
    const Type src = variant.type;
    if (src == type) {
        return;
    }
    assert(VM_TYPE_NUMBER(src)&&"Only support cast conv basic type.");
    assert(VM_TYPE_NUMBER(type));
    if (type == TYPE_DOUBLE) {
        PixtronVM_ConvertToDoubleValue(&variant);
    } else if (type == TYPE_LONG) {
        PixtronVM_ConvertToLongValue(&variant);
    } else {
        variant.type = type;
    }
    PixtronVM_PushOperand(context, &variant);
}

static void PixtronVM_Store(RuntimeContext *context) {
    Variant variant;
    PixtronVM_PopOperand(context, &variant);
    const uint8_t subOps = PixtronVM_ReadByteCodeU8(context);
    const DataSource s = OPS_DATA_SOURCE(subOps);
    const guint16 index = PixtronVM_ReadByteCodeU16(context);
    // Global variable
    if (s == GLOBAL_VAR) {
        PixtronVM_SetKlassFileValue(context, index, &variant);
    }
    // Local variable
    else if (s == LOCAL_VAR) {
        PixtronVM_SetLocalTable(context, index, &variant);
    }
}


static void PixtronVM_Load(RuntimeContext *context) {
    Variant variant;
    PixtronVM_GetOpsData(context, &variant);
    PixtronVM_PushOperand(context, &variant);
}

static void PixtronVM_ThrowException(RuntimeContext *context, gchar *fmt, ...) {
    const VirtualStackFrame *frame = context->frame;
    const Method *method = frame->method;
    const gchar *threadName = g_thread_get_name(g_thread_self());
    gchar *methodName = method->toString(method);
    va_list vaList;
    va_start(vaList, fmt);
    const gchar *message = g_strdup_printf(fmt, &vaList);
    va_end(vaList);
    g_printerr(
        "\n*******************************************************************************\n"
        "*                    VIRTUAL MACHINE CRASH - FATAL ERROR                       *\n"
        "*******************************************************************************\n"
        "* Thread: %-60s *\n"
        "* Method: (%p) %-60s *\n"
        "*                                                                             *\n"
        "* [ERROR]: %-64s*\n"
        "*                                                                             *\n"
        "* Execution State:                                                            *\n"
        "*   Program Counter: 0x%08x (instruction #%d)                         *\n"
        "*   Stack Pointer:   0x%08x (depth: %d/%d)                             *\n"
        "*   Locals Address:  0x%p                                    *\n"
        "*   Operand Stack:   0x%p                                    *\n"
        "*                                                                             *\n"
        "*******************************************************************************\n\n",
        threadName ? threadName : "<unnamed-thread>",
        method,
        methodName,
        message, // error message
        frame->pc, // pc
        frame->pc, // pc
        frame->sp, // sp
        frame->sp, // stack deep
        frame->maxStackSize, // max stack deep
        (void *) frame->locals, // local
        (void *) frame->operandStack // operand stack address
    );
    g_free(methodName);
    g_free((gpointer) message);
    g_thread_exit(NULL);
}


extern Variant *PixtronVM_CallMethod(const Method *method) {
    RuntimeContext *context = PixotronVM_calloc(sizeof(RuntimeContext));
    context->vm = method->klass->vm;
    VirtualStack *stack = PixotronVM_calloc(sizeof(VirtualStack));
    stack->maxDepth = VM_MAX_STACK_DEPTH;
    stack->depth = 0;
    context->stack = stack;
    context->throwException = PixtronVM_ThrowException;
    PixtronVM_PushStackFrame(context, method);

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
                PixtronVM_Cmp(context, ops);
                break;
            case CONV:
                PixtronVM_CONV(context);
                break;
            case RET:
                PixtronVM_PopStackFrame(context);
                break;
            case CALL:
                break;
            default:
                context->throwException(context, "Unsupported opcode: %02x", ops);
        }
    }
}
