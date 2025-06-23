#include <errno.h>
#include <time.h>
#include <stdio.h>

#include "itype.h"
#include "kni.h"
#include "istring.h"

void System_currentTimeNano(RuntimeContext *context, KniValue *result) {
    struct timespec ts;
    if (timespec_get(&ts, TIME_UTC) != TIME_UTC) {
        context->throwException(context, "Failed to get current time:%s.", strerror(errno));
    }
    pvm_kni_set_long(result, ts.tv_nsec);
}

void System_printNanoDiff(RuntimeContext *context) {
    const long pre = pvm_kni_get_long(context, 0);
    const long now = pvm_kni_get_long(context, 1);
    printf("%ld ns\n", (now - pre));
}


void System_println(RuntimeContext *context) {
    const KniValue *value = pvm_kni_get_object(context, 0);
    g_assert(value->type == TYPE_STRING);
    if (value->obj == NULL) {
        printf("null\n");
        return;
    }
    const String *str = value->obj;
    printf("%s\n", str->str);
}
