#ifndef PTYPE_H
#define PTYPE_H
#include <glib.h>
#include <stdbool.h>
#include <stdint.h>
#include "pvm.h"

typedef struct _VMValue VMValue;

typedef struct _Method Method;

typedef struct _PStr {
    uint32_t len;
    uint32_t hash_code;

    union {
        char *value;
        char values[8];
    };
} PStr;


typedef struct {
    int index;
    PStr *name;
} Symbol;

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


typedef struct {
    const Method *method;
    uint16_t argc;
    const VMValue **argv;
} CallMethodParam;

typedef struct {
    Type type;
    Symbol *name;
} MethodParam;

struct _Method;

typedef struct _Method {
    Symbol *name;
    Symbol *cleanup;
    Type ret;
    uint16_t max_stacks;
    uint16_t max_locals;
    uint32_t offset;
    uint32_t end_offset;
    uint16_t argc;
    MethodParam *argv;
    struct _Klass *klass;
    bool native_func;
    bool raw_str;
    void *native_handle;
    // Native resource cleanup method
    Method *m_cleanup;

    char * (*toString)(const struct _Method *);
} Method;

typedef struct {
    Symbol *name;
    VMValue *value;
} Field;

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


typedef struct _Klass {
    // Magic
    uint32_t magic;
    // Version
    Version version;
    Symbol *file;
    // Constants size
    uint32_t const_size;
    // Constants
    PStr **constants;
    // Class methods
    Method **methods;
    // Class bytecode
    uint8_t *bytecode;
    // Field meta data
    Field *fields;
    //Class name
    Symbol *name;
    // Field count
    uint32_t fcount;
    // Method count
    uint32_t mcount;
    const struct VM *vm;
    Symbol *library;
    // Constructor
    Method *constructor;
} Klass;

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

typedef struct VM {
    // Klass path
    char *klassPath;
    // Environments
    GHashTable *envs;
    // Classes
    GHashTable *klasses;
    // String constant pool
    GHashTable *string_table;
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


extern inline char *pvm_get_symbol_value(const Symbol *symbol);


extern inline bool pvm_symbol_equal(const Symbol *a, const Symbol *b);


extern inline bool pvm_symbol_cmp_cstr(const Symbol *symbol, const char *str);


extern inline Symbol *pvm_symbol_dup(const Symbol *symbol);


#define VM_VALUE_SIZE sizeof(VMValue)

#endif
