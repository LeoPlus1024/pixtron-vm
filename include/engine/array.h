#ifndef ARRAY_H
#define ARRAY_H
#include "itype.h"

typedef struct {
    Type type;
    uint8_t *data;
    uint8_t length;
} Array;

#define INDEX_OUT_OF_BOUND_CHECK(context,array,index) if (index >= array.length) pvm_throw_exception(context,"Array index out of bound:: requested #%d (valid range: 0-%d)",index,array->length);

extern inline Array *pvm_new_array(Type type, uint32_t length);

extern inline void pvm_array_value_set(const Array *array, int index, const VMValue *value);
#endif //ARRAY_H
