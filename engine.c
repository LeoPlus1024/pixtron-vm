#include "engine.h"

#include <assert.h>
#include <config.h>
#include <stdbool.h>

#include "bc_reader.h"
#include "klass.h"
#include "memory.h"
#include "stack.h"
#include <dlfcn.h>

#include "istring.h"
#include "kni.h"

#ifdef VM_DEBUG_ENABLE
#include "op_gen.h"
#endif


static inline void pvm_exec_canonical_binary_operation(RuntimeContext *context, const Opcode opcode) {
    const VMValue *source_operand = pvm_pop_operand(context);
    VMValue *target_operand = pvm_get_operand(context);

    const Type t0 = target_operand->type;
    const Type t1 = source_operand->type;
    if (t0 != t1) {
        context->throwException(
            context,
            "Operand type mismatch the first operand is:'%s' and the second operand is:'%s'.",
            TYPE_NAME[t0],
            TYPE_NAME[t1]
        );
    }

    if (opcode == ADD) {
        APPLY_COMPOUND_OPERATOR(target_operand, source_operand, +, context);
    } else if (opcode == SUB) {
        APPLY_COMPOUND_OPERATOR(target_operand, source_operand, -, context);
    } else if (opcode == MUL) {
        APPLY_COMPOUND_OPERATOR(target_operand, source_operand, *, context);
    } else if (opcode == DIV) {
        if (TYPE_SMALL_INTEGER(t1) && source_operand->i32 == 0 || TYPE_BIGGER_INTEGER(t1) && source_operand->i64 == 0) {
            context->throwException(context, "Divisor cannot be zero.");
        }
        APPLY_COMPOUND_OPERATOR(target_operand, source_operand, /, context);
    } else {
        context->throwException(context, "Canonical binary operation '%02x' is not supported.", opcode);
    }
}

static inline void pvm_goto(RuntimeContext *context) {
    VirtualStackFrame *frame = context->frame;
    const int16_t offset = (int16_t) pvm_bytecode_read_u16(context);
    // Offset contain current opcode
    frame->pc = frame->pc + offset - 2;
}

static inline void pvm_ifeq_ifnq(RuntimeContext *context, const Opcode opcode) {
    const VMValue *value = pvm_pop_operand(context);
    const bool ifeq = opcode == IFEQ;
    const int16_t offset = (int16_t) pvm_bytecode_read_u16(context);
    VirtualStackFrame *frame = context->frame;
    if ((value->i32 == 0) == ifeq) {
        frame->pc = frame->pc + offset - 2;
    }
}

static inline void pvm_less_granter_than_equal(RuntimeContext *context, Opcode opcode) {
    const int16_t offset = (int16_t) pvm_bytecode_read_u16(context);
    const VMValue *value = pvm_pop_operand(context);
    const int32_t flag = value->i32;
    bool jump_branch = false;

    switch (opcode) {
        case IFLE: jump_branch = (flag <= 0);
            break;
        case IFLT: jump_branch = (flag < 0);
            break;
        case IFGE: jump_branch = (flag >= 0);
            break;
        case IFGT: jump_branch = (flag > 0);
            break;
        default: break;
    }

    if (jump_branch) {
        context->frame->pc += (offset - 2);
    }
}

extern inline void pvm_cmp(RuntimeContext *context, const Opcode opcode) {
    const VMValue *source_operand = pvm_pop_operand(context);
    VMValue *targe_operand = pvm_get_operand(context);
    switch (opcode) {
        case ICMP:
            targe_operand->i32 = SIGN_CMP(targe_operand->i32, source_operand->i32);
            break;
        case DCMP:
            targe_operand->i32 = SIGN_CMP(targe_operand->f64, source_operand->f64);
            break;
        case LCMP:
            targe_operand->i32 = SIGN_CMP(targe_operand->i64, source_operand->i64);
            break;
        default:
            context->throwException(context, "unsupported cmp opcode:%02x", opcode);
    }
    targe_operand->type = TYPE_INT;
}

static void pvm_conv(RuntimeContext *context, const Opcode opcode) {
    VMValue *value = pvm_get_operand(context);
    if (opcode == I2F || opcode == L2F) {
        pvm_value_to_double(value);
    } else if (opcode == I2L || opcode == F2L) {
        pvm_value_to_long(value);
    } else if (opcode == F2I || opcode == L2I) {
        pvm_value_to_int(value);
    }
}

static void pvm_store(RuntimeContext *context) {
    const VMValue *value = pvm_pop_operand(context);
    const uint8_t subOps = pvm_bytecode_read_u8(context);
    const DataSource s = OPS_DATA_SOURCE(subOps);
    const uint16_t index = pvm_bytecode_read_u16(context);
    // Global variable
    if (s == FIELD) {
        pvm_set_klass_field(context, index, value);
    }
    // Local variable
    else if (s == LOCAL) {
        pvm_set_local_value(context, index, value);
    }
}


static inline void pvm_load(RuntimeContext *context) {
    VMValue value;
    const uint8_t subOps = pvm_bytecode_read_u8(context);
    const DataSource source = OPS_DATA_SOURCE(subOps);
    const Type type = OPS_DATA_TYPE(subOps);
    // Immediate
    if (source == IMM) {
        pvm_bytecode_read_imm(context, type, &value);
    } else {
        const uint16_t index = pvm_bytecode_read_u16(context);
        // Local
        if (source == LOCAL) {
            pvm_get_local_value(context, index, &value);
        } else if (source == CONST) {
            pvm_get_klass_constant(context, index, &value);
        } else {
            pvm_get_klass_field(context, index, &value);
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
    pvm_push_operand(context, &value);
}

extern void pvm_thrown_exception(RuntimeContext *context, char *fmt, ...) {
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
        frame->max_stacks, // max stack deep
        (void *) frame->locals, // local
        (void *) frame->operand_stack // operand stack address
    );
    g_free(methodName);
    g_free((gpointer) message);
    g_thread_exit(NULL);
}

static inline VMValue *pvm_ret(RuntimeContext *context) {
    VirtualStackFrame *frame = context->frame;
    const VMValue *value = NULL;
    const bool hasRetVal = frame->method->ret != TYPE_VOID;
    if (hasRetVal) {
        value = pvm_pop_operand(context);
    }
    VMValue *retVal = NULL;
    pvm_pop_stack_frame(context);
    if (frame->pre == NULL) {
        context->exit = exit;
        retVal = pvm_mem_cpy(value,VM_VALUE_SIZE);
    } else if (value != NULL) {
        pvm_push_operand(context, value);
    }
    pvm_stack_fram_dispose(&frame);
    return retVal;
}

static inline VMValue *pvm_call(RuntimeContext *context) {
    const uint16_t index = pvm_bytecode_read_u16(context);
    const Method *method = pvm_get_method(context, index);
    pvm_push_stack_frame(context, method);
    if (method->native_func) {
        const bool callWithRet = method->ret != TYPE_VOID;
        if (callWithRet) {
            const KniResultOperation ffiFunc = (KniResultOperation) method->native_handle;
            VMValue retVal;
            ffiFunc(context, &retVal);
            pvm_push_operand(context, &retVal);
        } else {
            const KniBaseOperation ffiFunc = (KniBaseOperation) (method->native_handle);
            ffiFunc(context);
        }
        // Native function execute after manual call ret instruction
        return pvm_ret(context);
    }
    return NULL;
}

static void inline pvm_assert(RuntimeContext *context) {
#ifdef VM_DEBUG_ENABLE
    const uint16_t index = pvm_bytecode_read_u16(context);
    VMValue value;
    pvm_get_klass_constant(context, index, &value);
    if (value.type != TYPE_STRING) {
        context->throwException(context, "Except a string type but it is '%s'", TYPE_NAME[value.type]);
    }
    const char *message;
    const String *obj = (String *) value.obj;
    if (obj != NULL) {
        message = obj->str;
    } else {
        message = "??";
    }
    context->throwException(context, "Assert fail:%s", message);
#else
    context->frame->pc += 3;
#endif
}


static void inline pvm_ishx(RuntimeContext *context, Opcode opcode) {
    const VMValue *source_operand = pvm_pop_operand(context);
    VMValue *target_operand = pvm_get_operand(context);
    if (target_operand->type != TYPE_INT || source_operand->type != TYPE_INT) {
        context->throwException(context, "ishl instruction only support small integer.");
    }
    const int32_t bit = source_operand->i32 & 0x1F;

    switch (opcode) {
        case IUSHR: {
            uint32_t value = (uint32_t) target_operand->i32;
            value = value >> bit;
            target_operand->i32 = (int32_t) value;
            break;
        }
        case ISHR:
            target_operand->i32 = target_operand->i32 >> bit;
            break;
        case ISHL:
            target_operand->i32 = target_operand->i32 << bit;
            break;
        default: ;
    }
}

static void inline pvm_lshx(RuntimeContext *context, Opcode opcode) {
    const VMValue *source_operand = pvm_pop_operand(context);
    VMValue *target_operand = pvm_get_operand(context);
    if (TYPE_BIGGER_INTEGER(target_operand->type) || source_operand->type != TYPE_INT) {
        context->throwException(
            context, "lshl instruction the first operand must is long and the second operand must is integer.");
    }
    const int32_t bit = source_operand->i32 & 0x3F;
    const int64_t value = target_operand->i64;
    switch (opcode) {
        case LUSHR: {
            uint64_t tmp = (uint64_t) value;
            tmp >>= bit;
            target_operand->i64 = (int64_t) tmp;
            break;
        }
        case LSHR:
            target_operand->i64 = value >> bit;
            break;
        case LSHL:
            target_operand->i64 = value << bit;
            break;
        default: ;
    }
}


extern void pvm_call_method(const CallMethodParam *callMethodParam) {
    const Method *method = callMethodParam->method;
    RuntimeContext *context = pvm_mem_calloc(sizeof(RuntimeContext));
    context->vm = method->klass->vm;
    context->exit = false;
    context->maxStackDepth = VM_MAX_STACK_DEPTH;
    context->stackDepth = 0;
    context->throwException = pvm_thrown_exception;
    const VMValue **args = callMethodParam->args;
    pvm_create_stack_frame(context, method, callMethodParam->argv, args);

    VMValue *retVal = NULL;
    while (!context->exit) {
        const Opcode opcode = pvm_bytecode_read_u8(context);
#ifdef VM_DEBUG_ENABLE
        const Method *m = context->frame->method;
        g_debug("%s : %s", m->toString(m), pvm_opcode_name(opcode));
#endif
        switch (opcode) {
            case LOAD:
                pvm_load(context);
                break;
            case STORE:
                pvm_store(context);
                break;
            case ADD:
            case SUB:
            case MUL:
            case DIV:
                pvm_exec_canonical_binary_operation(context, opcode);
                break;
            case GOTO:
                pvm_goto(context);
                break;
            case IFEQ:
            case IFNE:
                pvm_ifeq_ifnq(context, opcode);
                break;
            case IFLE:
            case IFLT:
            case IFGE:
            case IFGT:
                pvm_less_granter_than_equal(context, opcode);
                break;
            case ICMP:
            case DCMP:
            case LCMP:
                pvm_cmp(context, opcode);
                break;
            case I2L:
            case I2F:
            case L2I:
            case L2F:
            case F2I:
            case F2L:
                pvm_conv(context, opcode);
                break;
            case RET:
                retVal = pvm_ret(context);
                break;
            case POP:
                pvm_pop_operand(context);
                break;
            case CALL:
                retVal = pvm_call(context);
                break;
            case ASSERT:
                pvm_assert(context);
                break;
            case ISHL:
            case ISHR:
            case IUSHR:
                pvm_ishx(context, opcode);
                break;
            case LSHL:
            case LSHR:
            case LUSHR:
                pvm_lshx(context, opcode);
                break;
            default:
                context->throwException(context, "Unsupported opcode: %02x", opcode);
        }
    }
    g_thread_exit(retVal);
}
