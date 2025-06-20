#ifndef TYPE_H
#define TYPE_H

#include "Opcode.h"
#include <stdint.h>
#include <glib.h>
#include <stdbool.h>

typedef struct {
    union {
        int8_t i8;
        int16_t i16;
        int32_t i32;
        int64_t i64;
        double f64;
        void *obj;
    };

    Type type;
} VMValue;

typedef union {
    int8_t i8;
    int16_t i16;
    int32_t i32;
    int64_t i64;
    double f64;
} VMSmallValue;

struct VM;

typedef struct {
    Type type;
    char *name;
} MethodParam;

struct _Method;

typedef struct _Method {
    char *name;
    Type retType;
    uint16_t maxStackSize;
    gushort maxLocalsSize;
    uint32_t offset;
    uint32_t endOffset;
    uint16_t argv;
    MethodParam *args;
    struct _Klass *klass;
    bool nativeFunc;
    char **libName;
    uint16_t libCount;

    char * (*toString)(const struct _Method *);
} Method;

typedef struct {
    char *name;
    VMValue *value;
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
} BinaryHeader;


typedef struct VM {
    // Klass path
    gchar *klassPath;
    // Classes
    GHashTable *klassTable;
} PixtronVM;


struct _RuntimeContext;

typedef struct _RuntimeContext {
    volatile bool exit;
    // Current stack depth
    uint32_t stackDepth;
    // Max stack depth
    uint32_t maxStackDepth;
    // Stack top frame
    VirtualStackFrame *frame;
    // VM pointer
    const PixtronVM *vm;

    // Exception throw callback
    void (*throwException)(struct _RuntimeContext *context, gchar *fmt, ...);
} RuntimeContext;


typedef struct {
    const Method *method;
    uint16_t argv;
    const VMValue **args;
} CallMethodParam;


#define VM_VALUE_SIZE (sizeof(VMValue))

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
#endif //TYPE_H
