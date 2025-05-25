#include "include/Engine.h"

#include <assert.h>

#include "VirtualStack.h"

extern inline void PixotronVM_exec_add(PixtronVMPtr vm) {
    Variant a;
    Variant b;
    PixtronVM_stack_pop(vm, &a);
    PixtronVM_stack_pop(vm, &b);
    const DataType type = a.type;
    assert((type == b.type));
    assert(VM_TYPE_NUMBER(type));
    if (type == TYPE_BYTE) {
        const int8_t sum = a.value.b + b.value.b;
        b.value.b = sum;
    } else if (type == TYPE_INT) {
        const int32_t sum = a.value.i + b.value.i;
        b.value.i = sum;
    } else if (type == TYPE_LONG) {
        const int64_t sum = (a.value.l + b.value.l) & 0xffffffffffff;
        b.value.l = sum;
    } else {
        const double sum = a.value.d + b.value.d;
        b.value.d = sum;
    }
    PixtronVM_stack_push(vm, &b);
}
