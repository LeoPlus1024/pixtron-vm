#ifndef ENGINE_H
#define ENGINE_H
#include "VM.h"

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
        default: \
            (targetOperand)->f64 compoundOp##= (sourceOperand)->f64; \
            break; \
    } \
} while(0)

extern VMValue PixtronVM_CallMethod(const Method *method);

#endif //ENGINE_H
