#include "Kni.h"
#include "Engine.h"
#include "VMString.h"
#include "Memory.h"

static inline KniValue *Kni_GetParam(RuntimeContext *context, const uint16_t index) {
    g_assert(context!=NULL);
    const VirtualStackFrame *frame = context->frame;
    const Method *method = frame->method;
    const uint16_t argv = method->argv;
    if (index >= argv) {
        Kni_ThrowException(context, "GetParam fail index exceed legal range.");
    }
    return frame->locals + index;
}

extern void Kni_ThrowException(RuntimeContext *context, char *fmt, ...) {
    g_assert(context!=NULL);
    va_list vaList;
    va_start(vaList, fmt);
    PixtronVM_ThrowException(context, fmt, vaList);
    va_end(vaList);
}


extern void Kni_SetLong(KniValue *result, const int64_t value) {
    g_assert(result!=NULL);
    result->type = TYPE_LONG;
    result->i64 = value;
}


extern void Kni_SetInt(KniValue *result, const int32_t value) {
    g_assert(result!=NULL);
    result->type = TYPE_INT;
    result->i32 = value;
}

extern void Kni_SetShort(KniValue *result, const int16_t value) {
    g_assert(result!=NULL);
    result->type = TYPE_SHORT;
    result->i16 = value;
}

extern void Kni_SetByte(KniValue *result, const int8_t value) {
    g_assert(result!=NULL);
    result->type = TYPE_BYTE;
    result->i8 = value;
}

extern void Kni_SetDouble(KniValue *result, const double value) {
    g_assert(result!=NULL);
    result->type = TYPE_DOUBLE;
    result->f64 = value;
}

extern void Kni_SetBool(KniValue *result, const bool value) {
    g_assert(result!=NULL);
    result->type = TYPE_BOOL;
    result->i8 = (int8_t) value;
}

extern int8_t Kni_GetByteParam(RuntimeContext *context, const uint16_t index) {
    const KniValue *param = Kni_GetParam(context, index);
    g_assert(param->type==TYPE_BYTE);
    return param->i8;
}

extern int16_t Kni_GetShortParam(RuntimeContext *context, const uint16_t index) {
    const KniValue *param = Kni_GetParam(context, index);
    g_assert(param->type==TYPE_SHORT);
    return param->i16;
}

extern int32_t Kni_GetIntParam(RuntimeContext *context, const uint16_t index) {
    const KniValue *param = Kni_GetParam(context, index);
    g_assert(param->type==TYPE_INT);
    return param->i32;
}

extern int64_t Kni_GetLongParam(RuntimeContext *context, const uint16_t index) {
    const KniValue *param = Kni_GetParam(context, index);
    g_assert(param->type==TYPE_LONG);
    return param->i64;
}

extern double Kni_GetDoubleParam(RuntimeContext *context, const uint16_t index) {
    const KniValue *param = Kni_GetParam(context, index);
    g_assert(param->type==TYPE_DOUBLE);
    return param->f64;
}

extern KniString *Kni_GetStringParam(RuntimeContext *context, const uint16_t index) {
    const KniValue *param = Kni_GetParam(context, index);
    g_assert(param->type==TYPE_STRING);
    const String *str = param->obj;
    const uint64_t len = str->len;
    if (str == NULL || len == 0) {
        return NULL;
    }
    char *buf = PixotronVM_calloc(len);
    memcpy(buf, str->str, str->len);
    KniString *text = PixotronVM_calloc(sizeof(KniString));
    text->len = str->len;
    text->buf = buf;
    return text;
}

extern KniValue *Kni_GetObjectParam(RuntimeContext *context, uint16_t index) {
    return Kni_GetParam(context, index);
}


extern void Kni_ReleaseKniString(KniString **string) {
    if (string == NULL || *string == NULL) {
        return;
    }
    KniString *self = *string;
    PixotronVM_free(TO_REF(self->buf));
    PixotronVM_free(CAST_REF(self));
}
