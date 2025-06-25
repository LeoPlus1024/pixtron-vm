#ifndef ENGINE_H
#define ENGINE_H
#include "itype.h"

#define OPS_DATA_TYPE(value) ((value >> 4) & 0x0f)

#define OPS_DATA_SOURCE(value) (value & 0x0f)

#define TYPE_BIGGER_INTEGER(value) (value == TYPE_LONG)

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
#define APPLY_COMPOUND_OPERATOR( \
    targetOperand, \
    sourceOperand, \
    compoundOp, \
    executionContext \
) \
do { \
    switch ((targetOperand)->type) { \
        case TYPE_BYTE: \
        case TYPE_SHORT: \
        case TYPE_INT: \
            (targetOperand)->i32 compoundOp##= (sourceOperand)->i32; \
            break; \
        case TYPE_LONG: \
            (targetOperand)->i64 compoundOp##= (sourceOperand)->i64; \
            break; \
        case TYPE_DOUBLE: \
            (targetOperand)->f64 compoundOp##= (sourceOperand)->f64; \
            break; \
        default:    \
            pvm_thrown_exception(executionContext ,"Not support compound operator."); \
    } \
} while(0)

extern void pvm_call_method(const CallMethodParam *callMethodParam);

extern void pvm_thrown_exception(RuntimeContext *context, char *fmt, ...);

#endif //ENGINE_H
