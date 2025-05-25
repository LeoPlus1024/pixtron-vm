#include <assert.h>

#include "PixotronVM.h"
#include "VirtualStack.h"

/**
 * ADD opcode test
 */
int main(void) {
    const uint8_t buffer[] = {
        204, 187, 170, 255, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 51, 15, 0, 0, 0, 0, 51, 15, 0, 0,
        0, 2
    };

    PixtronVMPtr vm = PixtronVM_create(buffer, sizeof(buffer) / sizeof(buffer[0]));
    PixtronVM_exec(vm);
    Variant variant;
    PixtronVM_stack_pop(vm, &variant);
    assert(variant.type==TYPE_INT);
    assert(variant.value.i == 30);
    PixtronVM_destroy(&vm);
    return 0;
}
