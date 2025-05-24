//
// Created by leo on 25-5-22.
//

#ifndef OPS_H
#define OPS_H
#include <stdint.h>

typedef enum:uint8_t {
    // 栈操作
    PUSH, // 压栈（操作数含类型和值）
    POP, // 弹栈（操作数含类型）
    CALL,

    // 算术运算
    OP_ADD, // 加法（操作数含类型）
    OP_SUB, // 减法
    OP_MUL,
    OP_DIV,

    // 类型转换
    OP_CONV, // 转换（如 i2l、f2d）

    // 控制流
    OP_GOTO,
    OP_IFEQ,
    OP_INVOKE, // 方法调用

    // 内存操作
    OP_LOAD,
    OP_STORE,

    // 扩展预留
    OP_RESERVED_START = 0x80
} Opcode;

// 类型标记（操作数中使用）
typedef enum:uint8_t {
    TYPE_BYTE = 0,
    TYPE_BOOL,
    TYPE_INT,
    TYPE_LONG,
    TYPE_DOUBLE,
    TYPE_REF,
} DataType;

typedef enum {
    IMPLIED = 0,
    LOCAL_VAR,
    GLOBAL_VAR,
    IMMEDIATE
} DataSource;

static const uint8_t TYPE_SIZE[] = {
    [TYPE_BYTE] = 1,
    [TYPE_BOOL] = 1,
    [TYPE_INT] = 4,
    [TYPE_LONG] = 6,
    [TYPE_DOUBLE] = 8,
    [TYPE_REF] = 6
};

#define OPS_COMPOSE(type,source) (source | (type << 4))
#define OPS_DATA_TYPE(value) ((value >> 4) & 0x0f)
#define OPS_DATA_SOURCE(value) (value & 0x0f)
#endif //OPS_H
