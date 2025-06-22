#include "FFI.h"
#include "Engine.h"
#include "String.h"

static inline FFIParam *FFI_GetParam(RuntimeContext *context, const uint16_t index) {
    g_assert(context!=NULL);
    const VirtualStackFrame *frame = context->frame;
    const Method *method = frame->method;
    const uint16_t argv = method->argv;
    if (index >= argv) {
        FFI_ThrowException(context, "GetParam fail index exceed legal range.");
    }
    return frame->locals + index;
}

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

extern void FFI_SetDouble(FFIResult *result, const double value) {
    g_assert(result!=NULL);
    result->type = TYPE_DOUBLE;
    result->f64 = value;
}

extern void FFI_SetBool(FFIResult *result, const bool value) {
    g_assert(result!=NULL);
    result->type = TYPE_BOOL;
    result->i8 = (int8_t) value;
}

extern int8_t FFI_GetByteParam(RuntimeContext *context, const uint16_t index) {
    const FFIParam *param = FFI_GetParam(context, index);
    g_assert(param->type==TYPE_BYTE);
    return param->i8;
}

extern int16_t FFI_GetShortParam(RuntimeContext *context, const uint16_t index) {
    const FFIParam *param = FFI_GetParam(context, index);
    g_assert(param->type==TYPE_SHORT);
    return param->i16;
}

extern int32_t FFI_GetIntParam(RuntimeContext *context, const uint16_t index) {
    const FFIParam *param = FFI_GetParam(context, index);
    g_assert(param->type==TYPE_INT);
    return param->i32;
}

extern int64_t FFI_GetLongParam(RuntimeContext *context, const uint16_t index) {
    const FFIParam *param = FFI_GetParam(context, index);
    g_assert(param->type==TYPE_LONG);
    return param->i64;
}

extern double FFI_GetDoubleParam(RuntimeContext *context, const uint16_t index) {
    const FFIParam *param = FFI_GetParam(context, index);
    g_assert(param->type==TYPE_DOUBLE);
    return param->f64;
}

extern char *FFI_GetStringParam(RuntimeContext *context, const uint16_t index) {
    const FFIParam *param = FFI_GetParam(context, index);
    g_assert(param->type==TYPE_STRING);
    const String *str = param->obj;
    if (str == NULL) {
        return NULL;
    }
    return str->str;
}
