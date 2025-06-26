#include <time.h>
#include <stdio.h>

#include "itype.h"
#include "kni.h"
#include "istring.h"

int64_t currentTimeNano() {
    struct timespec ts;
    if (timespec_get(&ts, TIME_UTC) != TIME_UTC) {
        return -1;
    }
    return ts.tv_nsec;
}

void printNanoDiff(RuntimeContext *context) {
    const long pre = pvm_kni_get_long(context, 0);
    const long now = pvm_kni_get_long(context, 1);
    printf("%ld ns\n", (now - pre));
}


void println(RuntimeContext *context) {
    const KniValue *value = pvm_kni_get_object(context, 0);
    g_assert(value->type == TYPE_STRING);
    if (value->obj == NULL) {
        printf("null\n");
        return;
    }
    const String *str = value->obj;
    printf("%s\n", str->str);
}
