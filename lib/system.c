#include <stdio.h>

#include "ptype.h"
#include "pvm.h"

int64_t currentTimeNano() {
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        return -1;
    }
    return ts.tv_sec * 1000000000LL + ts.tv_nsec;;
}

void printNanoDiff(const int64_t base, const int64_t now) {
    printf("%ld ns\n", (now - base));
}


void println(const Value *value) {
    const Type type = pvm_get_value_type(value);
    switch (type) {
        case TYPE_BYTE:
            printf("%d\n", pvm_value_get_byte(value));
            break;
        case TYPE_SHORT:
            printf("%d\n", pvm_value_get_short(value));
            break;
        case TYPE_INT:
            printf("%d\n", pvm_value_get_int(value));
            break;
        case TYPE_LONG:
            printf("%ld\n", pvm_value_get_long(value));
            break;
        case TYPE_STRING:
            char *str = pvm_value_get_string(value);
            printf("%s\n", str);
            pvm_free_string(&str);
            break;
        case TYPE_BOOL:
            printf("%s\n", pvm_value_get_bool(value) ? "true" : "false");
            break;
        default:
            printf("%p\n", value->obj);
    }
}
