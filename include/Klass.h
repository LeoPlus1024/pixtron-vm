#ifndef CLASS_H
#define CLASS_H
#include "Type.h"

typedef struct {
    Type type;
    gchar *name;
} FuncParam;

typedef struct {
    uint8_t *name;
    Type retType;
    uint32_t offset;
    FuncParam *params;
} Func;

typedef struct {
    uint32_t magic;
    Version version;
    Func **farr;
    uint8_t *codes;
    Variant *varr;
    gchar *name;
    guint vlen;
} Klass;

extern Klass *PixtronVM_class_get(const PixtronVM *vm, const gchar *klassName);

#endif //CLASS_H
