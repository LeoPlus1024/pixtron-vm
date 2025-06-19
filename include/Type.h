#ifndef TYPE_H
#define TYPE_H

#include "Opcode.h"
#include <stdint.h>
#include <glib.h>

typedef union {
    int8_t i8;
    int16_t i16;
    int32_t i32;
    int64_t i64;
    double f64;

    struct {
        uint64_t payload: 48;
        Type type: 16;
    };
} VMValue;

struct VM;

typedef struct {
    Type type;
    gchar *name;
} MethodParam;

struct _Method;

typedef struct _Method {
    gchar *name;
    Type retType;
    gushort maxStackSize;
    gushort maxLocalsSize;
    uint32_t offset;
    uint32_t endOffset;
    gushort paramCount;
    MethodParam *params;
    struct _Klass *klass;

    gchar * (*toString)(const struct _Method *);
} Method;

typedef struct {
    Type type;
    gchar *name;
} Field;

typedef struct _Klass {
    // Magic
    uint32_t magic;
    // Version
    Version version;
    // Class methods
    Method *methods;
    // Class bytecode
    uint8_t *byteCode;
    // Field meta data
    Field *fields;
    // Field value
    VMValue *fieldVals;
    //Class name
    char *name;
    // Field count
    uint32_t fieldCount;
    // Method count
    uint32_t methodCount;
    const struct VM *vm;
} Klass;

struct _VirtualStackFrame;

typedef struct _VirtualStackFrame {
    // Method
    const Method *method;
    // local variable
    VMValue *locals;
    // Operand stack
    VMValue *operandStack;
    // Program counter
    uint32_t pc;
    // Stack pointer
    uint32_t sp;
    uint32_t maxStackSize;
    uint32_t maxLocalsSize;
    // Previous stack frame
    struct _VirtualStackFrame *pre;
} VirtualStackFrame;

typedef struct {
    // 魔数
    uint32_t magic;
    // 版本号
    Version version;
    // 数据偏移量
    uint32_t dataOffset;
    // 数据长度
    uint32_t dataLength;
    // 代码偏移量
    uint32_t codeOffset;
} BinaryHeader, *BinaryHeaderPtr;


typedef struct VM {
    // Klass path
    gchar *klassPath;
    // Classes
    GHashTable *klassTable;
} PixtronVM;


struct _RuntimeContext;

typedef struct _RuntimeContext {
    // Current stack depth
    uint32_t stackDepth;
    // Max stack depth
    uint32_t maxStackDepth;
    // Stack top frame
    VirtualStackFrame *frame;
    // VM pointer
    const PixtronVM *vm;

    void (*throwException)(struct _RuntimeContext *context, gchar *fmt, ...);
} RuntimeContext;


#define VM_VALUE_SIZE (sizeof(VMValue))

/**
 * @brief Creates a VM value object from a byte buffer
 *
 * Constructs a virtual machine value object by parsing the given byte buffer according
 * to the specified type descriptor. Typically used for deserializing network data
 * or persisted byte streams.
 *
 * @param type Type descriptor defining the data structure layout of the buffer
 * @param buf Source byte buffer pointer. Must be valid and conform to type format.
 * @return VMValue Constructed virtual machine value object
 *
 * @note Caller must ensure:
 * 1. Buffer lifetime extends at least until function returns
 * 2. Buffer format matches the type structure defined by type parameter
 * 3. For variable-length types, buffer header contains required length metadata
 *
 * @warning No boundary checking is performed. Buffer validity must be verified
 *          before invocation.
 *
 * @example
 * const uint8_t packet[] = {0x01, 0x00, 0x00, 0x3F, 0x80};
 * VMValue v = PixtronVM_CreateValueFromBuffer(DOUBLE, packet);
 */
extern inline VMValue PixtronVM_CreateValueFromBuffer(Type type, const uint8_t *buf);

/**
 * Convert a Variant to double-precision floating point representation
 * @param value Pointer to the value to convert (modified in-place)
 */
extern inline void PixtronVM_ConvertToDoubleValue(VMValue *value);

/**
 * Convert a Variant to 64-bit integer (long) representation
 * @param value Pointer to the value to convert (modified in-place)
 */
extern inline void PixtronVM_ConvertToLongValue(VMValue *value);


extern inline void PixtronVM_ConvertToIntValue(VMValue *value);


/**
 *<pre>
 * NaN-boxing pattern (64-bit):
 * [ Sign  |  Exponent   | Quiet NaN | Payload (Type + Data) ]
 *  1 bit    11 bits       1 bit          51 bits
 *          0x7FF           1
 *
 * Type tag occupies bits 61-58 (0xA000000000000000 >> 48 = 0xA)
 * 0x7FF0000000000000 mask isolates NaN control bits
 *</pre>
 * @brief Extracts type information from VMValue's binary representation
 *
 * Decodes the VMValue's type using its internal bit pattern:
 * - Values not matching the NaN boxing pattern (0x7FF0_0000_0000_0000 mask)
 *   are considered primitive doubles (TYPE_DOUBLE)
 * - NaN-boxed values store type tags in bits [61:48] (0x0F mask after shifting)
 * <pre>
 *
 * @param value VM value to inspect. Passed by const-value for optimization.
 * @return Type Decoded type tag, either TYPE_DOUBLE or a NaN-boxed type ID
 *
 * @note Implementation relies on:
 * 1. IEEE 754 NaN boxing implementation details
 * 2. Little-endian memory layout
 * 3. 48-bit type tag storage (values 0-15)
 *
 * @warning Platform-dependent:
 * - Assumes 64-bit VMValue representation
 * - Depends on specific NaN quiet bit patterns
 * - Type tags may change across VM versions
 *
 * @see VMValue Encoding Specification Section 3.2
 *      "NaN-boxed Type Tagging"
 *
 * @example
 * VMValue num = 3.1415926; // Raw double
 * Type t = PixtronVM_GetValueType(num); // Returns TYPE_DOUBLE
 *
 * VMValue obj = 0x7FF1A00000000000; // NaN-boxed type 0xA
 * Type t = PixtronVM_GetValueType(obj); // Returns 0xA
 */
extern inline Type PixtronVM_GetValueType(const VMValue *value);

#endif //TYPE_H
