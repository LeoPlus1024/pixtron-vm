#include "Engine.h"

#include <assert.h>
#include <Config.h>
#include <stdbool.h>

#include "ByteCodeReader.h"
#include "Klass.h"
#include "Memory.h"
#include "VirtualStack.h"

static inline void PixtronVM_CheckOperandType(RuntimeContext *context, const VMValue *targetOperand,
                                              const VMValue *sourceOperand) {
    const Type t0 = targetOperand->type;
    const Type t1 = sourceOperand->type;
    if (t0 == t1) {
        return;
    }
    context->throwException(context, "Operand type mismatch the first operand is:'%s' and the second operand is:'%s'.",
                            TYPE_NAME[t0], TYPE_NAME[t1]);
}


static inline VMValue PixtronVM_GetOpsData(RuntimeContext *context, VMValue *value) {
    const guint8 subOps = PixtronVM_ReadByteCodeU8(context);
    const Type type = OPS_DATA_TYPE(subOps);
    const DataSource source = OPS_DATA_SOURCE(subOps);
    // Immediate
    if (source == IMMEDIATE) {
        PixtronVM_ReadByteCodeImm(context, value);
    } else {
        const guint16 index = PixtronVM_ReadByteCodeU16(context);
        const bool local = source == LOCAL_VAR;
        // Local
        if (local) {
            PixtronVM_GetLocalTable(context, index, value);
        }
        // Klass field
        else {
            PixtronVM_GetKlassFileValue(context, index, value);
        }
    }
}

static inline void PixtronVM_executeCanonicalBinaryOperation(RuntimeContext *context, Opcode opcode) {
    VMValue targetOperand;
    VMValue sourceOperand;
    targetOperand = *PixtronVM_PopOperand(context);
    sourceOperand = *PixtronVM_PopOperand(context);
    PixtronVM_CheckOperandType(context, &targetOperand, &sourceOperand);
    if (opcode == ADD) {
        APPLY_COMPOUND_OPERATOR(targetOperand, sourceOperand, +, context);
    } else if (opcode == SUB) {
        APPLY_COMPOUND_OPERATOR(targetOperand, sourceOperand, -, context);
    } else if (opcode == MUL) {
        APPLY_COMPOUND_OPERATOR(targetOperand, sourceOperand, *, context);
    } else if (opcode == DIV) {
        APPLY_COMPOUND_OPERATOR(targetOperand, sourceOperand, /, context);
    } else {
        context->throwException(context, "Canonical binary operation '%02x' is not supported.", opcode);
    }
    PixtronVM_PushOperand(context, &targetOperand);
}

static void PixtronVM_CheckCon(RuntimeContext *context, const Opcode opcode) {
    const gint16 offset = (gint16) PixtronVM_ReadByteCodeU16(context);
    if (opcode != GOTO) {
        const VMValue *value = PixtronVM_PopOperand(context);
        const bool ifeq = opcode == IFEQ;
        assert(VM_TYPE_BOOL(value->type)&&"Ifeq or Ifnq only support boolean.");
        if (!((ifeq && VM_FALSE(value->i8)) || !ifeq && VM_TRUE(value->i8))) {
            return;
        }
    }
    VirtualStackFrame *frame = context->frame;
    // Offset contain current opcode
    frame->pc = frame->pc + offset - 2;
}


extern inline void PixtronVM_Cmp(RuntimeContext *context, Opcode opcode) {
    VMValue *top;
    VMValue *next;
    top = PixtronVM_PopOperand(context);
    next = PixtronVM_PopOperand(context);
    switch (opcode) {
        case ICMP:
            next->i32 = SIGN_CMP(next->i32, top->i32);
            break;
        case DCMP:
            next->i32 = SIGN_CMP(next->f64, top->f64);
            break;
        case LCMP:
            next->i32 = SIGN_CMP(next->f64, top->f64);
            break;
        default:
            context->throwException(context, "unsupported cmp opcode:%02x", opcode);
    }
    PixtronVM_PushOperand(context, next);
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
    VMValue value;
    PixtronVM_GetOpsData(context, &value);
    PixtronVM_PushOperand(context, &value);
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
        const Opcode opcode = PixtronVM_ReadByteCodeU8(context);

        switch (opcode) {
            case LOAD:
                PixtronVM_Load(context);
                break;
            case STORE:
                PixtronVM_Store(context);
                break;
            case ADD:
            case SUB:
            case MUL:
            case DIV:
                PixtronVM_executeCanonicalBinaryOperation(context, opcode);
                break;
            case GOTO:
            case IFEQ:
            case IFNE:
                PixtronVM_CheckCon(context, opcode);
                break;
            case ICMP:
            case DCMP:
            case LCMP:
                PixtronVM_Cmp(context, opcode);
                break;
            case CONV:
                PixtronVM_CONV(context);
                break;
            case RET:
                PixtronVM_PopStackFrame(context);
                break;
            case POP:
                PixtronVM_PopOperand(context,NULL);
                break;
            case CALL:
                break;
            default:
                context->throwException(context, "Unsupported opcode: %02x", opcode);
        }
    }
}
