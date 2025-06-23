#ifndef KNI_H
#define KNI_H
#include <stdbool.h>
#include <stdint.h>

typedef struct _VMValue KniValue;
typedef struct _RuntimeContext RuntimeContext;

typedef struct {
    uint64_t len;
    char *buf;
} KniString;

/**
 * Base function pointer type for all context-aware Kni operations.
 *
 * Represents the minimal operation signature that requires runtime context
 * but produces no result and accepts no parameters. All operations must
 * conform to this base contract.
 *
 * @param context Mandatory runtime execution environment. Provides access to:
 *   - VM state and registers
 *   - Memory management
 *   - Error reporting mechanisms
 *   - System interfaces
 *
 * Implementation requirements:
 *   1. MUST validate context pointer before access
 *   2. MUST report errors via context's error system
 *   3. SHOULD be reentrant and thread-safe
 */
typedef void (*KniBaseOperation)(RuntimeContext *context);

/**
 * Extended function pointer type for operations requiring result output.
 *
 * Inherits all requirements from KniBaseOperation while adding result
 * handling capabilities.
 *
 * @param context Mandatory runtime execution environment
 * @param result Optional pre-allocated result container. When provided:
 *   - MUST be populated with operation output
 *   - Ownership remains with caller
 *   - May be NULL for fire-and-forget operations
 *
 * Usage protocol:
 *   if (result != NULL) {
 *       // Populate result container
 *   } else {
 *       // Execute without output
 *   }
 *
 * Memory safety:
 *   - Implementation must not retain result reference
 *   - Caller responsible for result memory lifecycle
 */
typedef void (*KniResultOperation)(RuntimeContext *context, KniValue *result);

/**
 * Throws a runtime exception with formatted error message.
 *
 * This function registers an exception in the execution context using
 * a printf-style formatted message. The exception will interrupt normal
 * control flow and propagate through the runtime stack until caught.
 *
 * @param context Runtime execution context where exception is registered
 * @param fmt Format string compatible with printf semantics
 * @param ... Variadic arguments matching the format specifiers
 *
 * Memory management:
 *   - Internally allocates memory for formatted message
 *   - Context becomes owner of exception memory
 *   - Caller should not free the format string
 *
 * Error propagation:
 *   - Sets context->exception_state = EXCEPTION_RAISED
 *   - Stores formatted message in context->exception_message
 *   - Preserves stack trace at throw point
 */
extern void Kni_ThrowException(RuntimeContext *context, char *fmt, ...);

/**
 * Sets a 64-bit signed integer value in Kni result container.
 *
 * @param result Pointer to preallocated KniResult structure
 * @param value 64-bit integer value to store
 *
 * Postconditions:
 *   - result->type = Kni_TYPE_LONG
 *   - result->value.long_value = value
 *   - result->size = sizeof(int64_t)
 */
extern void Kni_SetLong(KniValue *result, int64_t value);

/**
 * Sets a 32-bit signed integer value in Kni result container.
 *
 * @param result Pointer to initialized KniResult structure
 * @param value 32-bit integer value to store
 *
 * Postconditions:
 *   - result->type = Kni_TYPE_INT
 *   - result->value.int_value = value
 *   - result->size = sizeof(int32_t)
 */
extern void Kni_SetInt(KniValue *result, int32_t value);

/**
 * Sets a 16-bit signed integer value in Kni result container.
 *
 * @param result Pointer to initialized KniResult structure
 * @param value 16-bit integer value to store
 *
 * Postconditions:
 *   - result->type = Kni_TYPE_SHORT
 *   - result->value.short_value = value
 *   - result->size = sizeof(int16_t)
 */
extern void Kni_SetShort(KniValue *result, int16_t value);

/**
 * Sets an 8-bit signed integer value in Kni result container.
 *
 * @param result Pointer to initialized KniResult structure
 * @param value 8-bit integer value to store
 *
 * Postconditions:
 *   - result->type = Kni_TYPE_BYTE
 *   - result->value.byte_value = value
 *   - result->size = sizeof(int8_t)
 */
extern void Kni_SetByte(KniValue *result, int8_t value);

/**
 * Sets a double-precision floating point value in Kni result container.
 *
 * @param result Pointer to initialized KniResult structure
 * @param value Double-precision floating point value to store
 *
 * Postconditions:
 *   - result->type = Kni_TYPE_DOUBLE
 *   - result->value.double_value = value
 *   - result->size = sizeof(double)
 *
 * Note: Single-precision floats will be automatically promoted
 */
extern void Kni_SetDouble(KniValue *result, double value);

/**
 * Sets a boolean value in Kni result container.
 *
 * @param result Pointer to initialized KniResult structure
 * @param value Boolean value to store (true/false)
 *
 * Postconditions:
 *   - result->type = Kni_TYPE_BOOL
 *   - result->value.bool_value = value
 *   - result->size = sizeof(bool)
 *
 * Storage details:
 *   - Uses native bool representation (typically 1 byte)
 *   - Non-zero values considered true
 */
extern void Kni_SetBool(KniValue *result, bool value);

extern int8_t Kni_GetByteParam(RuntimeContext *context, uint16_t index);

extern int16_t Kni_GetShortParam(RuntimeContext *context, uint16_t index);

extern int32_t Kni_GetIntParam(RuntimeContext *context, uint16_t index);

extern int64_t Kni_GetLongParam(RuntimeContext *context, uint16_t index);

extern double Kni_GetDoubleParam(RuntimeContext *context, uint16_t index);

extern KniString *Kni_GetStringParam(RuntimeContext *context, uint16_t index);

extern KniValue *Kni_GetObjectParam(RuntimeContext *context, uint16_t index);


extern void Kni_ReleaseKniString(KniString **string);

#endif //KNI_H
