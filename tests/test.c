#include "pvm.h"

static void exec_main(const char *klass_name) {
    VM *vm = pvm_init("build");
    pvm_launch_main(vm, klass_name);
    pvm_destroy(&vm);
}


int main(int argc, char *argv[]) {
    exec_main("Fib");
    exec_main("Array");
    exec_main("Klass");
    exec_main("Bool");
    exec_main("Logic");
    exec_main("Number");
}
