// #include "kni.h"
// #include "engine.h"
// #include "istring.h"
// #include "memory.h"
//
// static inline KniValue *kni_get_param(RuntimeContext *context, const uint16_t index) {
//     g_assert(context!=NULL);
//     const VirtualStackFrame *frame = context->frame;
//     const Method *method = frame->method;
//     const uint16_t argv = method->argv;
//     if (index >= argv) {
//         pvm_kni_throw_exception(context, "GetParam fail index exceed legal range.");
//     }
//     return frame->locals + index;
// }
//
// extern void pvm_kni_throw_exception(RuntimeContext *context, char *fmt, ...) {
//     g_assert(context!=NULL);
//     va_list vaList;
//     va_start(vaList, fmt);
//     pvm_thrown_exception(context, fmt, vaList);
//     va_end(vaList);
// }
//
//
// extern void pvm_kni_set_long(KniValue *result, const int64_t value) {
//     g_assert(result!=NULL);
//     result->type = TYPE_LONG;
//     result->i64 = value;
// }
//
//
// extern void pvm_kni_set_int(KniValue *result, const int32_t value) {
//     g_assert(result!=NULL);
//     result->type = TYPE_INT;
//     result->i32 = value;
// }
//
// extern void pvm_kni_set_short(KniValue *result, const int16_t value) {
//     g_assert(result!=NULL);
//     result->type = TYPE_SHORT;
//     result->i16 = value;
// }
//
// extern void pvm_kni_set_byte(KniValue *result, const int8_t value) {
//     g_assert(result!=NULL);
//     result->type = TYPE_BYTE;
//     result->i8 = value;
// }
//
// extern void pvm_kni_set_double(KniValue *result, const double value) {
//     g_assert(result!=NULL);
//     result->type = TYPE_DOUBLE;
//     result->f64 = value;
// }
//
// extern void pvm_kni_set_bool(KniValue *result, const bool value) {
//     g_assert(result!=NULL);
//     result->type = TYPE_BOOL;
//     result->i8 = (int8_t) value;
// }
//
// extern int8_t pvm_kni_get_byte(RuntimeContext *context, const uint16_t index) {
//     const KniValue *param = kni_get_param(context, index);
//     g_assert(param->type==TYPE_BYTE);
//     return param->i8;
// }
//
// extern int16_t pvm_kni_get_short(RuntimeContext *context, const uint16_t index) {
//     const KniValue *param = kni_get_param(context, index);
//     g_assert(param->type==TYPE_SHORT);
//     return param->i16;
// }
//
// extern int32_t pvm_kni_get_int(RuntimeContext *context, const uint16_t index) {
//     const KniValue *param = kni_get_param(context, index);
//     g_assert(param->type==TYPE_INT);
//     return param->i32;
// }
//
// extern int64_t pvm_kni_get_long(RuntimeContext *context, const uint16_t index) {
//     const KniValue *param = kni_get_param(context, index);
//     g_assert(param->type==TYPE_LONG);
//     return param->i64;
// }
//
// extern double pvm_kni_get_double(RuntimeContext *context, const uint16_t index) {
//     const KniValue *param = kni_get_param(context, index);
//     g_assert(param->type==TYPE_DOUBLE);
//     return param->f64;
// }
//
// extern KniString *pvm_kni_get_str(RuntimeContext *context, const uint16_t index) {
//     const KniValue *param = kni_get_param(context, index);
//     g_assert(param->type==TYPE_STRING);
//     const String *str = param->obj;
//     const uint64_t len = str->len;
//     if (str == NULL || len == 0) {
//         return NULL;
//     }
//     char *buf = pvm_mem_calloc(len);
//     memcpy(buf, str->str, str->len);
//     KniString *text = pvm_mem_calloc(sizeof(KniString));
//     text->len = str->len;
//     text->buf = buf;
//     return text;
// }
//
// extern KniValue *pvm_kni_get_object(RuntimeContext *context, uint16_t index) {
//     return kni_get_param(context, index);
// }
//
//
// extern void pvm_kni_release_str(KniString **string) {
//     if (string == NULL || *string == NULL) {
//         return;
//     }
//     KniString *self = *string;
//     pvm_mem_free(TO_REF(self->buf));
//     pvm_mem_free(CAST_REF(self));
// }
