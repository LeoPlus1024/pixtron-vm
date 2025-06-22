#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "VM.h"

static uint64_t currentTimeNanos() {
    struct timespec ts;
    if (timespec_get(&ts, TIME_UTC) != TIME_UTC) {
        printf("Get timespec fail:%s\n", strerror(errno));
        exit(0);
    }
    return ts.tv_nsec;
}


void testMathMax(const int a, const int b, const int expect) {
    VM *vm = PixtronVM_CreateVM("build");
    Value *av = PixtronVM_CreateInt(a);
    Value *bv = PixtronVM_CreateInt(b);
    const Value *args[2] = {
        av,
        bv
    };
    const uint64_t t0 = currentTimeNanos();
    Value *value = PixtronVM_LaunchVM(vm, "TMath", "testMax", 2, args);
    const uint64_t t1 = currentTimeNanos();
    const int32_t rs = PixtronVM_GetInt(value);
    printf("Math.Max(%d, %d) = %d, time: %ld ns\n", a, b, rs, t1 - t0);
    assert(rs == expect);
    PixtronVM_FreeValue(&value);
    PixtronVM_DestroyVM(&vm);
}

void testHelloWorld() {
    VM *vm = PixtronVM_CreateVM("build");
    PixtronVM_LaunchVM(vm, "TString", "printHelloWorld", 0, NULL);
    PixtronVM_DestroyVM(&vm);
}

int main(int argc, char *argv[]) {
    testMathMax(10, 11, 11);
    testHelloWorld();
}
