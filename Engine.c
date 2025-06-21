#include "Engine.h"

#include <assert.h>
#include <Config.h>
#include <stdbool.h>

#include "ByteCodeReader.h"
#include "Klass.h"
#include "Memory.h"
#include "Stack.h"
#include <dlfcn.h>
#include <FFI.h>


static inline void PixtronVM_executeCanonicalBinaryOperation(RuntimeContext *context, const Opcode opcode) {
    const VMValue *sourceOperand = PixtronVM_PopOperand(context);
    VMValue *targetOperand = PixtronVM_PopOperand(context);

    const Type t0 = targetOperand->type;
    const Type t1 = sourceOperand->type;
    if (t0 != t1) {
        context->throwException(
            context,
            "Operand type mismatch the first operand is:'%s' and the second operand is:'%s'.",
            TYPE_NAME[t0],
            TYPE_NAME[t1]
        );
    }

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
    PixtronVM_MoveStackFrameSp(context, -1);
}

static inline void PixtronVM_Goto(RuntimeContext *context) {
    VirtualStackFrame *frame = context->frame;
    const int16_t offset = (int16_t) PixtronVM_ReadByteCodeU16(context);
    // Offset contain current opcode
    frame->pc = frame->pc + offset - 2;
}

static inline void PixtronVM_Ifeqne(RuntimeContext *context, const Opcode opcode) {
    const VMValue *value = PixtronVM_PopOperand(context);
    const bool ifeq = opcode == IFEQ;
    const int16_t offset = (int16_t) PixtronVM_ReadByteCodeU16(context);
    VirtualStackFrame *frame = context->frame;
    if ((value->i32 == 0) == ifeq) {
        frame->pc = frame->pc + offset - 2;
    }
}

static inline void PixtronVM_LessGranterThanEqual(RuntimeContext *context, Opcode opcode) {
    const int16_t offset = (int16_t) PixtronVM_ReadByteCodeU16(context);
    const VMValue *value = PixtronVM_PopOperand(context);
    const int32_t flag = value->i32;
    bool jump2Branch = false;

    switch (opcode) {
        case IFLE: jump2Branch = (flag <= 0);
            break;
        case IFLT: jump2Branch = (flag < 0);
            break;
        case IFGE: jump2Branch = (flag >= 0);
            break;
        case IFGT: jump2Branch = (flag > 0);
            break;
        default: break;
    }

    if (jump2Branch) {
        context->frame->pc += (offset - 2);
    }
}

extern inline void PixtronVM_Cmp(RuntimeContext *context, const Opcode opcode) {
    const VMValue *sourceOperand = PixtronVM_PopOperand(context);
    VMValue *targetOperand = PixtronVM_PopOperand(context);
    switch (opcode) {
        case ICMP:
            targetOperand->i32 = SIGN_CMP(targetOperand->i32, sourceOperand->i32);
            break;
        case DCMP:
            targetOperand->i32 = SIGN_CMP(targetOperand->f64, sourceOperand->f64);
            break;
        case LCMP:
            targetOperand->i32 = SIGN_CMP(targetOperand->i64, sourceOperand->i64);
            break;
        default:
            context->throwException(context, "unsupported cmp opcode:%02x", opcode);
    }
    targetOperand->type = TYPE_INT;
    PixtronVM_MoveStackFrameSp(context, -1);
}

static void PixtronVM_CONV(RuntimeContext *context, Opcode opcode) {
    VMValue *value = PixtronVM_PopOperand(context);
    if (opcode == I2F || opcode == L2F) {
        PixtronVM_ConvertToDoubleValue(value);
    } else if (opcode == I2L || opcode == F2L) {
        PixtronVM_ConvertToLongValue(value);
    } else if (opcode == F2I || opcode == L2I) {
        PixtronVM_ConvertToIntValue(value);
    }
    PixtronVM_PushOperand(context, NULL);
}

static void PixtronVM_Store(RuntimeContext *context) {
    const VMValue *value = PixtronVM_PopOperand(context);
    const uint8_t subOps = PixtronVM_ReadByteCodeU8(context);
    const DataSource s = OPS_DATA_SOURCE(subOps);
    const uint16_t index = PixtronVM_ReadByteCodeU16(context);
    // Global variable
    if (s == GLOBAL_VAR) {
        PixtronVM_SetKlassFileValue(context, index, value);
    }
    // Local variable
    else if (s == LOCAL_VAR) {
        PixtronVM_SetLocalTable(context, index, value);
    }
}


static inline void PixtronVM_Load(RuntimeContext *context) {
    VMValue value;
    const uint8_t subOps = PixtronVM_ReadByteCodeU8(context);
    const DataSource source = OPS_DATA_SOURCE(subOps);
    const Type type = OPS_DATA_TYPE(subOps);
    // Immediate
    if (source == IMMEDIATE) {
        value.type = type;
        PixtronVM_ReadByteCodeImm(context, &value);
    } else {
        const uint16_t index = PixtronVM_ReadByteCodeU16(context);
        const bool local = source == LOCAL_VAR;
        // Local
        if (local) {
            PixtronVM_GetLocalTable(context, index, &value);
        }
        // Klass field
        else {
            PixtronVM_GetKlassFileValue(context, index, &value);
        }
        if (type != value.type) {
            context->throwException(
                context,
                "Load value type mismatch the expected type is:'%s' and the actual type is:'%s'",
                TYPE_NAME[type],
                TYPE_NAME[value.type]
            );
        }
    }
    PixtronVM_PushOperand(context, &value);
}

extern void PixtronVM_ThrowException(RuntimeContext *context, char *fmt, ...) {
    const VirtualStackFrame *frame = context->frame;
    const Method *method = frame->method;
    const gchar *threadName = g_thread_get_name(g_thread_self());
    gchar *methodName = method->toString(method);
    va_list vaList;
    va_start(vaList, fmt);
    const char *message = g_strdup_vprintf(fmt, vaList);
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
        context->stackDepth, // stack deep
        frame->maxStackSize, // max stack deep
        (void *) frame->locals, // local
        (void *) frame->operandStack // operand stack address
    );
    g_free(methodName);
    g_free((gpointer) message);
    g_thread_exit(NULL);
}

static inline VMValue *PixtronVM_Ret(RuntimeContext *context) {
    const VirtualStackFrame *frame = context->frame;
    const Type retType = frame->method->retType;
    VMValue *value = NULL;
    const bool hasRetVal = retType != TYPE_VOID;
    if (hasRetVal) {
        value = PixtronVM_PopOperand(context);
    }
    PixtronVM_PopStackFrame(context);
    if (frame->pre != NULL) {
        if (hasRetVal) {
            PixtronVM_PushOperand(context, value);
        }
        return NULL;
    }
    context->exit = true;
    if (!hasRetVal) {
        return NULL;
    }
    VMValue *retVal = PixotronVM_calloc(VM_VALUE_SIZE);
    memcpy(retVal, value, VM_VALUE_SIZE);
    return retVal;
}

static inline void PixtronVM_Call(RuntimeContext *context) {
    const char *methodName = PixtronVM_ReadByteCodeString(context);
    const VirtualStackFrame *frame = context->frame;
    const Method *method = PixtronVM_GetKlassMethod(frame->method->klass, methodName);
    if (method == NULL) {
        context->throwException(context, "Method '%s' not found.", methodName);
    }
    if (!method->nativeFunc) {
        PixtronVM_PushStackFrame(context, method);
    } else {
        void *handle = dlsym(RTLD_DEFAULT, method->name);
        const bool callWithRet = method->retType != TYPE_VOID;
        const int32_t argv = method->argv;
        if (argv > 0) {
            PixtronVM_MoveStackFrameSp(context, argv - 1);
        }
        if (callWithRet) {
            const FFIResultOperation ffiFunc = (FFIResultOperation) (handle);
            VMValue retVal;
            ffiFunc(context, &retVal);
            PixtronVM_PushOperand(context, &retVal);
        } else {
            const FFIBaseOperation ffiFunc = (FFIBaseOperation) (handle);
            ffiFunc(context);
        }
    }
}


extern void PixtronVM_CallMethod(const CallMethodParam *callMethodParam) {
    const Method *method = callMethodParam->method;
    RuntimeContext *context = PixotronVM_calloc(sizeof(RuntimeContext));
    context->vm = method->klass->vm;
    context->exit = false;
    context->maxStackDepth = VM_MAX_STACK_DEPTH;
    context->stackDepth = 0;
    context->throwException = PixtronVM_ThrowException;
    const VMValue **args = callMethodParam->args;
    PixtronVM_CreateStackFrame(context, method, callMethodParam->argv, args);

    VMValue *retVal = NULL;
    while (!context->exit) {
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
                PixtronVM_Goto(context);
                break;
            case IFEQ:
            case IFNE:
                PixtronVM_Ifeqne(context, opcode);
                break;
            case IFLE:
            case IFLT:
            case IFGE:
            case IFGT:
                PixtronVM_LessGranterThanEqual(context, opcode);
                break;
            case ICMP:
            case DCMP:
            case LCMP:
                PixtronVM_Cmp(context, opcode);
                break;
            case I2L:
            case I2F:
            case L2I:
            case L2F:
            case F2I:
            case F2L:
                PixtronVM_CONV(context, opcode);
                break;
            case RET:
                retVal = PixtronVM_Ret(context);
                break;
            case POP:
                PixtronVM_PopOperand(context);
                break;
            case CALL:
                PixtronVM_Call(context);
                break;
            default:
                context->throwException(context, "Unsupported opcode: %02x", opcode);
        }
    }
    g_thread_exit(retVal);
}
