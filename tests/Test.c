#include "vm.h"


void exec_main(const char *klass_name) {
    VM *vm = pvm_init("build");
    pvm_launch_main(vm, klass_name);
    pvm_destroy(&vm);
}


int main(int argc, char *argv[]) {
    exec_main("TString");
    exec_main("TLogic");
    exec_main("TMath");
}
