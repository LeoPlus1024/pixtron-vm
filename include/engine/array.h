#ifndef ARRAY_H
#define ARRAY_H
#include "ptype.h"

typedef struct {
    Type type;
    uint8_t *data;
    uint32_t length;
} Array;

#define ARRAY_INDEX_OUT_OF_BOUND_CHECK(array,index) (index < 0 || index >= array->length)

extern inline Array *pvm_new_array(Type type, uint32_t length);

extern inline void pvm_array_value_set(const Array *array, int index, const VMValue *value);


extern inline void *pvm_array_value_get(const Array *array, int index);
#endif //ARRAY_H
