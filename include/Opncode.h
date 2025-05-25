#ifndef OPS_H
#define OPS_H
#include <stdint.h>

typedef enum:uint8_t {
    // 栈操作
    PUSH, // 压栈（操作数含类型和值）
    POP, // 弹栈（操作数含类型）

    // 算术运算
    ADD, // 加法（操作数含类型）
    SUB, // 减法
    MUL,
    DIV,

    // 类型转换
    CONV, // 转换（如 i2l、f2d）

    // 控制流
    GOTO,
    IFEQ,
    INVOKE, // 方法调用

    CALL,

    // 扩展预留
    RESERVED_START = 0x80
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
#define VM_TYPE_NUMBER(type) (type == TYPE_BYTE || type == TYPE_INT || type == TYPE_LONG || type == TYPE_DOUBLE)
#endif //OPS_H
