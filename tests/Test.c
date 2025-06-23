#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "engine.h"
#include "vm.h"
#include <math.h>

static uint64_t currentTimeNanos() {
    struct timespec ts;
    if (timespec_get(&ts, TIME_UTC) != TIME_UTC) {
        printf("Get timespec fail:%s\n", strerror(errno));
        exit(0);
    }
    return ts.tv_nsec;
}


void testMathMax(const int a, const int b, const int expect) {
    VM *vm = pvm_init("build");
    Value *av = pvm_create_int_value(a);
    Value *bv = pvm_create_int_value(b);
    const Value *args[2] = {
        av,
        bv
    };
    const uint64_t t0 = currentTimeNanos();
    Value *value = pvm_launch(vm, "TMath", "testMax", 2, args);
    const uint64_t t1 = currentTimeNanos();
    const int32_t rs = pvm_value_get_int(value);
    printf("Math.Max(%d, %d) = %d, time: %ld ns\n", a, b, rs, t1 - t0);
    assert(rs == expect);
    pvm_free_values(args, 2);
    pvm_destroy(&vm);
}

void testPow(const double a, const int b) {
    VM *vm = pvm_init("build");
    Value *av = pvm_create_double_value(a);
    Value *bv = pvm_create_double_value(b);
    const Value *args[2] = {
        av,
        bv
    };
    Value *value = pvm_launch(vm, "TMath", "testPow", 2, args);
    assert(pvm_value_get_double(value) == pow(a,b));
    pvm_free_value(&value);
    pvm_free_values(args, 2);
    pvm_destroy(&vm);
}

void testHelloWorld() {
    VM *vm = pvm_init("build");
    pvm_launch(vm, "TString", "printHelloWorld", 0, NULL);
    pvm_destroy(&vm);
}

int main(int argc, char *argv[]) {
    testMathMax(10, 11, 11);
    testHelloWorld();
    testPow(10, 20);
}
