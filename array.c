#include "array.h"
#include "memory.h"
#include "pobject.h"

static inline void pvm_array_destructor(Array *array) {
    if (array->length == 0) {
        return;
    }
    pvm_mem_free(TO_REF(array->data));
}

extern inline Array *pvm_new_array(const Type type, const uint32_t length) {
    const uint8_t size = TYPE_SIZE[type];
    Array *array = pvm_object_new(sizeof(Array), (ObjectDestructor) pvm_array_destructor, 1);
    array->type = type;
    array->length = length;
    array->data = pvm_mem_calloc(size * length);
    return array;
}

extern inline void pvm_array_value_set(const Array *array, const int index, const VMValue *value) {
    const Type type = array->type;
    const void *from = &(value->i64);
    const uint8_t size = TYPE_SIZE[type];
    void *dst = array->data + index * size;
    memcpy(dst, from, size);
}


extern inline void *pvm_array_value_get(const Array *array, const int index) {
    const Type type = array->type;
    const uint8_t size = TYPE_SIZE[type];
    return array->data + index * size;
}
