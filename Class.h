//
// Created by leo on 25-6-15.
//

#ifndef CLASS_H
#define CLASS_H
#include "Type.h"

typedef struct {
    Type type,
    uint8_t *name;
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
    Func **func;
    uint8_t *codes;
    uint8_t *namespace;
} Class;

#endif //CLASS_H
