#ifndef ARRAY_H
#define ARRAY_H
#include "ptype.h"

typedef struct {
    Type type;
    uint8_t *data;
    uint32_t length;
} PArr;

#define ARRAY_INDEX_OUT_OF_BOUND_CHECK(array,index) (index < 0 || index >= array->length)

extern inline PArr *pvm_new_array(Type type, uint32_t length);

extern inline void pvm_array_value_set(const PArr *array, int index, const VMValue *value);


extern inline void *pvm_array_value_get(const PArr *array, int index);
#endif //ARRAY_H
