#ifndef TYPE_H
#define TYPE_H

#include <stddef.h>

#include "Opncode.h"
#include <stdint.h>

typedef uint64_t VMValue;

struct VirtualStackFrame;

typedef struct VirtualStackFrame {
    // 局部变量表
    VMValue *localVarTable;
    // 局部变量数量
    size_t maxLocals;
    // 操作数栈
    VMValue *operandStack;
    // 最大栈深度
    size_t maxStack;
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
    V1_0,
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
    // Program count
    uint64_t pc;
    // Program
    uint8_t *buffer;
    // Program size
    uint32_t size;
    // Data
    uint8_t *data;
    // VM Stack
    VirtualStackPtr stack;
    // Binary header
    BinaryHeaderPtr header;
} PixtronVM, *PixtronVMPtr, **PixtronVMRef;


typedef struct {
    DataType type;

    union {
        int8_t b;
        int16_t u;
        int32_t i;
        int64_t l;
        double d;
    } value;
} Variant;


#define VM_VALUE_SIZE (sizeof(VMValue))


extern inline DataType PixtronVM_typeof(VMValue value);


extern inline void PixtronVM_to_VMValue(const Variant *variant, uint8_t *value);

#endif //TYPE_H
