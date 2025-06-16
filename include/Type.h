#ifndef TYPE_H
#define TYPE_H

#include <stddef.h>

#include "Opcode.h"
#include <stdint.h>
#include <glib.h>

#include "Klass.h"

typedef uint64_t VMValue;

struct VirtualStackFrame;

typedef struct VirtualStackFrame {
    // Class
    Class *klass;
    // 局部变量表
    VMValue *localVarTable;
    // 局部变量数量
    uint16_t maxLocals;
    // 操作数栈
    VMValue *operandStack;
    // 最大栈深度
    uint16_t maxStack;
    // 操作数栈指针
    size_t sp;
    // 调用栈
    struct VirtualStackFrame *pre;
    // 返回地址
    size_t returnAddress;
} *VirtualStackFramePtr;

typedef struct {
    // 当前栈深度
    size_t depth;
    // 最大栈深度
    size_t maxDepth;
    // 当前栈顶栈帧
    VirtualStackFramePtr frame;
} VirtualStack, *VirtualStackPtr;


typedef enum:uint16_t {
    V1_0 = 1,
    V1_1,
    V1_2,
    V1_3,
    V1_4,
    V1_5,
    V1_6,
    V1_7,
} Version;

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


typedef struct {
    // Working directory
    gchar *workdir;
    // Program count
    uint64_t pc;
    // VM Stack
    VirtualStackPtr stack;
    // Classes
    GHashTable *klassTable;
} PixtronVM, *PixtronVMPtr, **PixtronVMRef;


typedef struct {
    Type type;
    gchar *name;

    union {
        int8_t b;
        int16_t s;
        int32_t i;
        int64_t l;
        double d;
    } value;
} Variant;


#define VM_VALUE_SIZE (sizeof(VMValue))

/**
 * Get the type of a VMValue
 * @param value The VMValue to check
 * @return The Type enumeration value representing the VMValue's type
 */
extern inline Type PixtronVM_typeof(VMValue value);

/**
 * Convert a Variant to its raw VMValue representation
 * @param variant Pointer to the source Variant to convert
 * @param value Output buffer to store the raw VMValue bytes (must have enough space)
 */
extern inline void PixtronVM_to_VMValue(const Variant *variant, uint8_t *value);

/**
 * Convert a Variant to double-precision floating point representation
 * @param variant Pointer to the Variant to convert (modified in-place)
 */
extern inline void PixtronVM_to_VMDouble(Variant *variant);

/**
 * Convert a Variant to 64-bit integer (long) representation
 * @param variant Pointer to the Variant to convert (modified in-place)
 */
extern inline void PixtronVM_to_VMLong(Variant *variant);

/**
 * Perform arithmetic negation on a Variant's value
 * @param variant Pointer to the Variant to negate (modified in-place)
 */
extern inline void PixtronVM_negative(Variant *variant);

/**
 * Convert a VMValue to a Variant
 * @param value The VMValue to convert
 * @param variant Output buffer to store the converted Variant (modified in-place)
 */
extern inline void PixtronVM_to_Variable(VMValue value, Variant *variant);

#endif //TYPE_H
