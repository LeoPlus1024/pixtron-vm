#include <stddef.h>

#include "VM.h"

int main(int argc, char *argv[]) {
    const char *klassPath = "/home/leo/CLionProjects/PixotronVM/assembler/example/build";
    VM *vm = PixtronVM_CreateVM(klassPath);
    Value *first = PixtronVM_CreateInt(10);
    Value *second = PixtronVM_CreateInt(20);
    Value *value = PixtronVM_LaunchVM(vm, "HelloWorld", 2, NULL);
    PixtronVM_FreeValue(&value);
    PixtronVM_DestroyVM(&vm);
}
