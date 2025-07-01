#ifndef ITYPE_H
#define ITYPE_H

#include "opcode.h"
#include <stdint.h>
#include <glib.h>
#include <stdbool.h>

typedef struct _VMValue {
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
    Type ret;
    uint16_t max_stacks;
    uint16_t max_locals;
    uint32_t offset;
    uint32_t end_offset;
    uint16_t argv;
    MethodParam *args;
    struct _Klass *klass;
    bool native_func;
    char *lib_name;
    void *native_handle;

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
    // Constants size
    uint32_t const_size;
    // Constants
    VMValue **constants;
    // Class methods
    Method **methods;
    // Class bytecode
    uint8_t *bytecode;
    // Field meta data
    Field *fields;
    //Class name
    char *name;
    // Field count
    uint32_t fieldCount;
    // Method count
    uint32_t method_count;
    const struct VM *vm;
    char *library;
} Klass;

struct _VirtualStackFrame;

typedef struct _VirtualStackFrame {
    // Method
    const Method *method;
    uint32_t pc;
    // Stack pointer
    uint32_t sp;
    uint32_t max_stacks;
    uint32_t max_locals;
    // Point to klass bytecode area
    uint8_t *bytecode;
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
    char *klassPath;
    // Environments
    GHashTable *envs;
    // Classes
    GHashTable *klasses;
    // String constant pool
    GHashTable *string_constants;
} PixtronVM;


struct _RuntimeContext;

typedef struct _RuntimeContext {
    volatile bool exit;
    // Current stack pointer
    uint32_t sp;
    // Max stack depth
    uint32_t stack_size;
    // Current stack depth
    uint32_t stack_depth;
    // Stack top frame
    VirtualStackFrame *frame;
    // VM pointer
    const PixtronVM *vm;
    // Stack
    const uint8_t *stack;
    // Exception throw callback
    void (*throw_exception)(struct _RuntimeContext *context, gchar *fmt, ...);
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
extern inline void pvm_value_to_double(VMValue *value);

/**
 * Convert a Variant to 64-bit integer (long) representation
 * @param value Pointer to the value to convert (modified in-place)
 */
extern inline void pvm_value_to_long(VMValue *value);


extern inline void pvm_value_to_int(VMValue *value);


extern inline uint8_t pvm_load_typed_value_from_buffer(Type type, VMValue *value, const uint8_t *buf);
#endif //ITYPE_H
