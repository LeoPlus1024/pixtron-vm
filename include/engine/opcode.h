#ifndef OPCODE_H
#define OPCODE_H
#include <stdint.h>

typedef enum:uint8_t {
    #include "opcode.txt"
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
    TYPE_VOID,
    TYPE_STRING
} Type;

typedef enum {
    IMPLIED = 0,
    LOCAL,
    FIELD,
    IMM,
    CONST,
} DataSource;

static const uint8_t TYPE_SIZE[] = {
    [NIL] = 0,
    [TYPE_BYTE] = 1,
    [TYPE_SHORT] = 2,
    [TYPE_INT] = 4,
    [TYPE_LONG] = 8,
    [TYPE_DOUBLE] = 8,
    [TYPE_BOOL] = 1,
    [TYPE_REF] = 8
};

static const char *const TYPE_NAME[] = {
    "null",
    "byte",
    "short",
    "int",
    "long",
    "double",
    "bool",
    "object",
    "void",
    "string"
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

#endif
