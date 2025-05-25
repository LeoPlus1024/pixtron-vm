#include "include/Engine.h"

#include <assert.h>
#include <stdbool.h>

#include "VirtualStack.h"


extern inline void PixotronVM_exec_add_sbc(PixtronVMPtr vm,bool sbc) {
    Variant top;
    Variant next;

    PixtronVM_stack_pop(vm, &top);
    PixtronVM_stack_pop(vm, &next);

    assert(VM_TYPE_NUMBER(top.type) && "Top operand must be number.");
    assert(VM_TYPE_NUMBER(next.type) && "Next operand must be number.");

    if (sbc) {
        PixtronVM_negative(&top);
    }

    // 类型相同时直接运算
    if (top.type == next.type) {
        switch (top.type) {
#define HANDLE_CASE(type, op) \
        case type: { \
            next.value.op += top.value.op; \
            break; \
        }
            HANDLE_CASE(TYPE_BYTE, b)
            HANDLE_CASE(TYPE_SHORT, s)
            HANDLE_CASE(TYPE_INT, i)
            case TYPE_LONG:
                next.value.l = (next.value.l + top.value.l) & 0xffffffffffff;
                break;
            HANDLE_CASE(TYPE_DOUBLE, d)
#undef HANDLE_CASE
            default:
                assert(false && "Unhandled numeric type");
        }
    }
    // Double自动扩展
    else if (VM_TYPE_DOUBLE(top.type) || VM_TYPE_DOUBLE(next.type)) {
        PixtronVM_to_VMDouble(&top);
        PixtronVM_to_VMDouble(&next);
        const double sum = next.value.d + top.value.d;
        next.value.d = sum;
    }
    // Long自动扩展
    else if (VM_TYPE_LONG(top.type) || VM_TYPE_LONG(next.type)) {
        PixtronVM_to_VMLong(&top);
        PixtronVM_to_VMLong(&next);
        next.value.l = (next.value.l + top.value.l) & 0xffffffffffff;
    }
    // Byte、Short、Int
    else {
        next.value.i = next.value.i + top.value.i;
        if (VM_TYPE_BYTE(top.type) || VM_TYPE_BYTE(next.type)) {
            next.type = TYPE_BYTE;
        } else if (VM_TYPE_SHORT(top.type) || VM_TYPE_SHORT(next.type)) {
            next.type = TYPE_SHORT;
        } else {
            next.type = TYPE_INT;
        }
    }
    PixtronVM_stack_push(vm, &next);
}
