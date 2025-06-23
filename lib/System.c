#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

#include "Type.h"
#include "Kni.h"
#include "VMString.h"

void System_currentTimeNano(RuntimeContext *context, KniValue *result) {
    struct timespec ts;
    if (timespec_get(&ts, TIME_UTC) != TIME_UTC) {
        context->throwException(context, "Failed to get current time:%s.", strerror(errno));
    }
    Kni_SetLong(result, ts.tv_nsec);
}

void System_printNanoDiff(RuntimeContext *context) {
    const long pre = Kni_GetLongParam(context, 0);
    const long now = Kni_GetLongParam(context, 1);
    printf("%ld ns\n", (now - pre));
}


void System_println(RuntimeContext *context) {
    const KniValue *value = Kni_GetObjectParam(context, 0);
    g_assert(value->type == TYPE_STRING);
    if (value->obj == NULL) {
        printf("null\n");
        return;
    }
    const String *str = value->obj;
    printf("%s\n", str->str);
}
