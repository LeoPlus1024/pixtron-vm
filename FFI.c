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
    VMValue *val = (VMValue *) result;
    val->type = TYPE_LONG;
    val->i64 = value;
}

extern FFIParam FFI_GetParam(RuntimeContext *context, const uint16_t index) {
    g_assert(context!=NULL);
    const VirtualStackFrame *frame = context->frame;
    const Method *method = frame->method;
    if (index >= method->argv) {
        FFI_ThrowException(context, "GetParam fail index exceed legal range.");
    }
    const uint32_t sp = frame->sp - index;
    return (FFIParam) frame->operandStack + sp;
}


extern void FFI_SetInt(FFIResult *result, int32_t value) {
    g_assert(result!=NULL);
    VMValue *val = (VMValue *) result;
    val->type = TYPE_INT;
    val->i32 = value;
}

extern void FFI_SetShort(FFIResult *result, int16_t value) {
    g_assert(result!=NULL);
    VMValue *val = (VMValue *) result;
    val->type = TYPE_SHORT;
    val->i16 = value;
}

extern void FFI_SetByte(FFIResult *result, int8_t value) {
    g_assert(result!=NULL);
    VMValue *val = (VMValue *) result;
    val->type = TYPE_BYTE;
    val->i8 = value;
}

extern void FFI_SetFloat(FFIResult *result, double value) {
    g_assert(result!=NULL);
    VMValue *val = (VMValue *) result;
    val->type = TYPE_FLOAT;
    val->f64 = value;
}

extern void FFI_SetBool(FFIResult *result, bool value) {
    g_assert(result!=NULL);
    VMValue *val = (VMValue *) result;
    val->type = TYPE_BOOL;
    val->i8 = (int8_t) value;
}
