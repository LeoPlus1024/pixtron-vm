#include "PixotronVM.h"
#include "Type.h"

int main(int argc, char *argv[]) {
    const gchar *workDir = "/home/leo/CLionProjects/PixotronVM/assembler/example/build";
    PixtronVM *vm = PixtronVM_CreateVM(workDir);
    PixtronVM_LaunchVM(vm, "HelloWorld");
}
