#include "FFI.h"
#include "Engine.h"

extern void FFI_ThrowException(RuntimeContext *context, char *fmt, ...) {
    g_assert(context!=NULL);
    va_list vaList;
    va_start(vaList, fmt);
    PixtronVM_ThrowException(context, fmt, vaList);
    va_end(vaList);
}


extern void FFI_SetLong(FFIResult *result, const int64_t value) {
    g_assert(result!=NULL);
    result->type = TYPE_LONG;
    result->i64 = value;
}

extern FFIParam *FFI_GetParam(RuntimeContext *context, const uint16_t index) {
    g_assert(context!=NULL);
    const VirtualStackFrame *frame = context->frame;
    const Method *method = frame->method;
    if (index >= method->argv) {
        FFI_ThrowException(context, "GetParam fail index exceed legal range.");
    }
    const uint32_t sp = frame->sp - index;
    return frame->operandStack + sp;
}


extern void FFI_SetInt(FFIResult *result, const int32_t value) {
    g_assert(result!=NULL);
    result->type = TYPE_INT;
    result->i32 = value;
}

extern void FFI_SetShort(FFIResult *result, const int16_t value) {
    g_assert(result!=NULL);
    result->type = TYPE_SHORT;
    result->i16 = value;
}

extern void FFI_SetByte(FFIResult *result, const int8_t value) {
    g_assert(result!=NULL);
    result->type = TYPE_BYTE;
    result->i8 = value;
}

extern void FFI_SetFloat(FFIResult *result, const double value) {
    g_assert(result!=NULL);
    result->type = TYPE_FLOAT;
    result->f64 = value;
}

extern void FFI_SetBool(FFIResult *result, const bool value) {
    g_assert(result!=NULL);
    result->type = TYPE_BOOL;
    result->i8 = (int8_t) value;
}
