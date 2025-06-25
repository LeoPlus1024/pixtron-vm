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


void test_max(const int a, const int b) {
    VM *vm = pvm_init("build");
    Value *av = pvm_create_int_value(a);
    Value *bv = pvm_create_int_value(b);
    const Value *args[2] = {
        av,
        bv
    };
    Value *value = pvm_launch(vm, "TMath", "testMax", 2, args);
    const int32_t rs = pvm_value_get_int(value);
    assert(rs == ((a > b) ? a : b));
    pvm_free_values(args, 2);
    pvm_destroy(&vm);
}

void test_pow(const double a, const int b) {
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

void test_println_hello_world() {
    VM *vm = pvm_init("build");
    pvm_launch(vm, "TString", "printHelloWorld", 0, NULL);
    pvm_destroy(&vm);
}

void test_int_div(const int a, const int b) {
    VM *vm = pvm_init("build");
    const Value *args[] = {
        pvm_create_int_value(a),
        pvm_create_int_value(b)
    };
    Value *value = pvm_launch(vm, "TMath", "testIntDiv", 2, args);
    assert(pvm_value_get_int(value) == a / b);
    pvm_free_value(&value);
    pvm_destroy(&vm);
}

void test_double_div(const double a, const double b) {
    VM *vm = pvm_init("build");
    const Value *args[] = {
        pvm_create_double_value(a),
        pvm_create_double_value(b)
    };
    Value *value = pvm_launch(vm, "TMath", "testDoubleDiv", 2, args);
    assert(pvm_value_get_double(value) == a / b);
    pvm_free_value(&value);
    pvm_destroy(&vm);
}

void test_byte_add(const int8_t a, const int8_t b) {
    VM *vm = pvm_init("build");
    const Value *args[] = {
        pvm_create_byte_value(a),
        pvm_create_byte_value(b)
    };
    Value *value = pvm_launch(vm, "TMath", "testByteAdd", 2, args);
    const int8_t sum = pvm_value_get_byte(value);
    assert(sum == (int8_t)(a + b));
    pvm_free_value(&value);
    pvm_destroy(&vm);
}

int main(int argc, char *argv[]) {
    test_max(10, 11);
    test_println_hello_world();
    test_pow(10, 20);
    test_int_div(10, 5);
    test_double_div(10, 0.0);
    test_byte_add(127, 1);
}
