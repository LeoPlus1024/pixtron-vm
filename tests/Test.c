#include "PixotronVM.h"
#include "Type.h"

int main(int argc, char *argv[]) {
    const gchar *workDir = "/home/leo/CLionProjects/PixotronVM/assembler/example/build";
    PixtronVM *vm = PixtronVM_create(workDir);
    PixtronVM_exec(vm, "HelloWorld", "main");
}
