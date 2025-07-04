#ifndef ENGINE_H
#define ENGINE_H
#include "itype.h"

#define OPS_DATA_TYPE(value) ((value >> 4) & 0x0f)

#define OPS_DATA_SOURCE(value) (value & 0x0f)

#define TYPE_BIGGER_INTEGER(value) (value == TYPE_LONG)

#define TYPE_INTEGER(value) (value == TYPE_INT)

#define TYPE_SMALL_INTEGER(value) (value == TYPE_BYTE || value == TYPE_SHORT || value == TYPE_INT)
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
#define APPLY_COMPOUND_OPERATOR(target, source, op) \
do { \
    static const void* const jump_table[] = { \
        [TYPE_BYTE]    = &&small_integer, \
        [TYPE_SHORT]   = &&small_integer, \
        [TYPE_INT]     = &&small_integer, \
        [TYPE_LONG]    = &&bigger_integer, \
        [TYPE_DOUBLE]  = &&bigger_float \
    }; \
    goto *jump_table[(target)->type]; \
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
        const int16_t offset = (int16_t)pvm_bytecode_read_int16(context) - 2; \
        if (flag op 0) {                                                      \
            frame->pc += offset;                                              \
        }                                                                     \
    } while (0)

#define XCMP(context,op)    \
    do {                                                                            \
        const VMValue *source_operand = pvm_pop_operand(context);                   \
        VMValue *targe_operand = pvm_get_operand(context);                          \
        targe_operand->i32 = SIGN_CMP(targe_operand->op, source_operand->op);       \
        targe_operand->type = TYPE_INT;                                             \
    } while(0)

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

extern void pvm_call_method(const CallMethodParam *callMethodParam);

extern void pvm_thrown_exception(RuntimeContext *context, char *fmt, ...);

#endif //ENGINE_H
