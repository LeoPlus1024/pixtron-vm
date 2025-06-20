#include <errno.h>
#include <time.h>
#include <FFI.h>
#include <string.h>
#include "Type.h"

void VM_CurrentTimeNano(RuntimeContext *context, FFIResult result) {
    struct timespec ts;
    if (timespec_get(&ts, TIME_UTC) != TIME_UTC) {
        context->throwException(context, "Failed to get current time:%s.", strerror(errno));
    }
    FFI_SetLong(result, ts.tv_nsec);
}
