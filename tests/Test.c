#include <assert.h>

#include "PixotronVM.h"
#include "VirtualStack.h"

#define ARRAY_SIZE(buffer) (sizeof(buffer) / sizeof(buffer[0]))

static PixtronVMPtr exec(uint8_t *buffer, const uint32_t length) {
    PixtronVMPtr vm = PixtronVM_create(buffer, length);
    PixtronVM_exec(vm);
    return vm;
}

static void execRet(uint8_t *buffer, const uint32_t length, Variant *variant) {
    PixtronVMPtr vm = PixtronVM_create(buffer, length);
    PixtronVM_exec(vm);
    PixtronVM_stack_pop(vm, variant);
    PixtronVM_destroy(&vm);
}

void Add() {
    uint8_t buffer[] = {
        204, 187, 170, 255, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 67, 156, 255,
        255, 255, 255, 255, 0, 67, 1, 0, 0, 0, 0, 0, 2
    };
    Variant variant;
    execRet(buffer, ARRAY_SIZE(buffer), &variant);
    assert(variant.type==TYPE_LONG);
    assert(variant.value.l == -99);
}

void Cmp() {
    uint8_t buffer[] = {
        204, 187, 170, 255, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 67, 156, 255,
        255, 255, 255, 255, 0, 67, 1, 0, 0, 0, 0, 0, 10
    };

    Variant variant;

    execRet(buffer, ARRAY_SIZE(buffer), &variant);

    assert(variant.type==TYPE_LONG);
    assert(variant.value.l == -1);
}

int main(int argc, char *argv[]) {
    Add();
    Cmp();
}
