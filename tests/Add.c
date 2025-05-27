#include <assert.h>

#include "PixotronVM.h"
#include "VirtualStack.h"

/**
 * ADD opcode test
 */
int main(void) {
    uint8_t buffer[] = {
        204, 187, 170, 255, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 67, 156, 255,
        255, 255, 255, 255, 0, 67, 1, 0, 0, 0, 0, 0, 2
    };


    PixtronVMPtr vm = PixtronVM_create(buffer, sizeof(buffer) / sizeof(buffer[0]));
    PixtronVM_exec(vm);
    Variant variant;
    PixtronVM_stack_pop(vm, &variant);
    assert(variant.type==TYPE_LONG);
    assert(variant.value.l == -99);
    PixtronVM_destroy(&vm);
    return 0;
}
