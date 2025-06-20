#include "../include/api/VM.h"

int main(int argc, char *argv[]) {
    const char *klassPath = "/home/leo/CLionProjects/PixotronVM/assembler/example/build";
    VM *vm = PixtronVM_CreateVM(klassPath);
    Value *arg0 = PixtronVM_CreateInt(10);
    Value *arg1 = PixtronVM_CreateByte(20);
    const Value *args[2];
    args[0] = arg0;
    args[1] = arg1;
    Value *retVal = PixtronVM_LaunchVM(vm, "HelloWorld", 2, args);
    PixtronVM_FreeValue(&retVal);
    PixtronVM_FreeValue(&arg0);
    PixtronVM_FreeValue(&arg1);
    PixtronVM_DestroyVM(&vm);
}
