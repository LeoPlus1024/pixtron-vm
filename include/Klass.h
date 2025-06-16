#ifndef KLASS_H
#define KLASS_H
#include "Type.h"

typedef struct {
    Type type;
    gchar *name;
} FuncParam;

typedef struct {
    uint8_t *name;
    Type ret;
    uint32_t offset;
    FuncParam *params;
} Func;

typedef struct {
    gchar * name;
    VMValue value;
} FieldMeta;

typedef struct {
    uint32_t magic;
    Version version;
    Func *methods;
    uint8_t *byteCode;
    FieldMeta *fieldMetas;
    VMValue *fieldValues;
    gchar *name;
    guint vlen;
} Klass;

extern Klass *PixtronVM_class_get(const PixtronVM *vm, const gchar *klassName);

#endif //KLASS_H
