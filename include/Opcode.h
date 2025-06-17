#ifndef OPCODE_H
#define OPCODE_H
#include <stdint.h>

typedef enum:uint8_t {
    // 栈操作
    LOAD, // 压栈（操作数含类型和值）
    STORE, // 弹栈（操作数含类型）
    // 算术运算
    ADD, // 加法（操作数含类型）
    SBC, // 减法
    MUL,
    DIV,
    // 无条件跳转
    GOTO,
    // 如果栈顶数字为0则跳转
    IFEQ,
    // 如果栈顶数字不为0则跳转
    IFNE,
    // 整数比较
    ICMP,
    // 长整形比较
    LCMP,
    // 浮点数比较
    DCMP,
    // 类型转换
    CONV, // 转换（如 i2l、f2d）
    CALL,
    // 扩展预留
    RESERVED_START = 0x80
} Opcode;

// 类型标记（操作数中使用）
typedef enum:uint16_t {
    NIL = 0,
    TYPE_BYTE,
    TYPE_SHORT,
    TYPE_INT,
    TYPE_LONG,
    TYPE_DOUBLE,
    TYPE_BOOL,
    TYPE_REF,
} Type;

typedef enum {
    IMPLIED = 0,
    LOCAL_VAR,
    GLOBAL_VAR,
    IMMEDIATE
} DataSource;

static const uint16_t TYPE_SIZE[] = {
    [NIL] = 0,
    [TYPE_BYTE] = 1,
    [TYPE_BYTE] = 2,
    [TYPE_INT] = 4,
    [TYPE_LONG] = 6,
    [TYPE_DOUBLE] = 8,
    [TYPE_BOOL] = 1,
    [TYPE_REF] = 6
};

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

#define OPS_COMPOSE(type,source) (source | (type << 4))
#define OPS_DATA_TYPE(value) ((value >> 4) & 0x0f)
#define OPS_DATA_SOURCE(value) (value & 0x0f)
#define VM_TYPE_NUMBER(type) (type == TYPE_BYTE || type == TYPE_SHORT || type == TYPE_INT || type == TYPE_LONG || type == TYPE_DOUBLE)
#define VM_TYPE_DOUBLE(type) (type == TYPE_DOUBLE)
#define VM_TYPE_LONG(type) (type == TYPE_LONG)
#define VM_TYPE_BOOL(type) (type == TYPE_BOOL)
#define VM_TYPE_REF(type) (type == TYPE_REF)
#define VM_TYPE_SHORT(type) (type == TYPE_SHORT)
#define VM_TYPE_INT(type) (type == TYPE_INT)
#define VM_TYPE_BYTE(type) (type == TYPE_BYTE)
#define VM_TRUE(variant) (variant.value.b == 1)
#define VM_FALSE(variant) (variant.value.b == 0)
/**
 * Cast c language long to PixotronVM long
 */
#define CLONG_TO_VLONG(value) ((int64_t)((uint64_t)value << 16) >> 16)
/**
 * Cast PixotronVM long to c language long type
 */
#define VLONG_TO_CLONG(value) ((int64_t) ((uint64_t)value << 16) >> 16)

#define SIGN_CMP(a, b) ((typeof(a))((a) > (b)) - ((typeof(a))((a) < (b))))


#endif
