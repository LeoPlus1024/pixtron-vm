#include "VM.h"
#include "Type.h"

int main(int argc, char *argv[]) {
    const gchar *workDir = "/home/leo/CLionProjects/PixotronVM/assembler/example/build";
    PixtronVM *vm = PixtronVM_CreateVM(workDir);
    VMValue *value = PixtronVM_LaunchVM(vm, "HelloWorld");
    PixtronVM_FreeVMValue(&value);
    PixtronVM_DestroyVM(&vm);
}
