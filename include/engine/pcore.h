#ifndef ENGINE_H
#define ENGINE_H
#include "ptype.h"

typedef enum:uint8_t {
#include "opcode.txt"
} Opcode;

#define NIL_VALUE ((const VMValue){ .type = TYPE_NIL, .obj = NULL })

#define TYPE_BIGGER_INTEGER(value) (value == TYPE_LONG)

#define TYPE_INTEGER(value) (value == TYPE_INT)

#define TYPE_SMALL_INTEGER(value) (value == TYPE_BYTE || value == TYPE_SHORT || value == TYPE_INT || value == TYPE_BOOL)

#define TYPE_REFERENCE(type) (type == TYPE_HANDLE || type == TYPE_STRING || type == TYPE_ARRAY)
/**
 * @brief Safe sign-aware comparison macro for two values of the same type.
 *
 * This macro compares two values (a, b) and returns:
 *   - 1 if (a > b),
 *   - -1 if (a < b),
 *   - 0 if (a == b).
 *
 * The result is type-cast to match the input type for safe comparison operations,
 * avoiding potential implicit type conversion issues.
 *
 * @param a First value to compare (any comparable type)
 * @param b Second value to compare (must be same type as 'a')
 * @return (typeof(a)) - The comparison result in the input value's type
 */
#define SIGN_CMP(a, b) ((typeof(a))((a) > (b)) - ((typeof(a))((a) < (b))))


/**
 * @brief Applies type-aware compound assignment operation with automatic type resolution
 * @param targetOperand [IN/OUT] Target operand object to be modified.
 *           Must contain 'type' field and 'value' union (i, l, d members)
 * @param sourceOperand [IN] Source operand object providing values.
 *           Must have compatible type with target operand
 * @param compoundOp Compound assignment operator token (e.g +=, -=, *=)
 * @param executionContext Runtime context for error handling
 *
 * @note Type handling rules:
 *   - Integral types (BYTE/SHORT/INT) use 32-bit integer operations
 *   - LONG uses 64-bit integer operations
 *   - DOUBLE uses double-precision floating point
 *   - Type mismatch throws exception with actual type code
 *
 * @warning Target operand modification has no rollback mechanism on failure
 */
#define APPLY_COMPOUND_OPERATOR(target, source, op, context)                                                \
do {                                                                                                        \
    if (VM_DEBUG_ENABLE && (target->type != source->type)) {                                                   \
        context->throw_exception(context, "Compound operator require two same type operand.");              \
    }                                                                                                       \
    static const void* const jump_table[] = {                \
        [TYPE_BYTE]    = &&small_integer,                    \
        [TYPE_SHORT]   = &&small_integer,                    \
        [TYPE_INT]     = &&small_integer,                    \
        [TYPE_LONG]    = &&bigger_integer,                   \
        [TYPE_DOUBLE]  = &&bigger_float                      \
    };                                                       \
    goto *jump_table[(target)->type];                        \
small_integer: \
    (target)->i32 op##= (source)->i32; \
    break; \
bigger_integer: \
    (target)->i64 op##= (source)->i64; \
    break; \
bigger_float: \
    (target)->f64 op##= (source)->f64; \
    break; \
} while(0)

#define IFXX(context, op)                                                     \
    do {                                                                      \
        const VMValue *value = pvm_pop_operand(context);                      \
        const int32_t flag = value->i32;                                      \
        VirtualStackFrame *frame = context->frame;                            \
        const uint32_t pc = frame->pc;                                        \
        if (flag op 0) {                                                      \
            int16_t offset  = pvm_bytecode_read_int16(context);               \
            frame->pc = pc + offset;                                          \
        } else {                                                              \
            frame->pc = pc +  2;                                              \
        }                                                                     \
    } while (0)

#define XCMP(context,op)    \
    do {                                                                            \
        const VMValue *source_operand = pvm_pop_operand(context);                   \
        VMValue *targe_operand = pvm_get_operand(context);                          \
        targe_operand->i32 = SIGN_CMP(targe_operand->op, source_operand->op);       \
        targe_operand->type = TYPE_INT;                                             \
    } while(0)

#define CMPX(context,op,except)    \
    do {                                                                            \
        const VMValue *source_operand = pvm_pop_operand(context);                   \
        VMValue *targe_operand = pvm_get_operand(context);                          \
        const int32_t value = SIGN_CMP(targe_operand->op, source_operand->op);      \
        targe_operand->i32 = value == except;                                         \
        targe_operand->type = TYPE_BOOL;                                            \
    } while(0)

#define LOAD_BOOL_CONST(context,op)                            \
    do {                                                    \
        VMValue *value = pvm_next_operand(context);         \
        value->i32 = op;                                    \
        value->type = TYPE_BOOL;                            \
    } while (0)


#define DISPATCH  do {                                                              \
        Opcode opcode = (Opcode)pvm_bytecode_read_int8(context);                    \
       if(VM_DEBUG_ENABLE) {                                                        \
            extern char *pvm_opcode_name(Opcode opcode);                            \
            const char *opcode_name = pvm_opcode_name(opcode);                      \
            const Method *method = context->frame->method;                          \
            const char *method_name = method->toString(method);                     \
            const uint32_t pc = context->frame->pc - 1;                             \
            g_debug("(%04d)%s => %s",pc,method_name,opcode_name);                   \
        }                                                                           \
        goto *opcode_table[opcode];                                                 \
    } while (0)


extern VMValue *pvm_execute_context(RuntimeContext *context, VirtualStackFrame *call_frame);

extern void pvm_execute(const CallMethodParam *callMethodParam);


extern RuntimeContext *pvm_execute_context_get();

#endif //ENGINE_H
