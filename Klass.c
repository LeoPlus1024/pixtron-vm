#include "Klass.h"

#include <gio/gio.h>

#include "Memory.h"
#include "IError.h"
#include "Type.h"
#include "StringUtil.h"

#define MAGIC (0xFFAABBCC)

static void PixtronVM_FreeKlass(Klass **klass) {
    if (*klass == NULL) {
        return;
    }
    const Klass *self = *klass;
    PixotronVM_free(TO_REF(self->name));
    PixotronVM_free(TO_REF(self->byteCode));
    gint fieldCount = (gint) self->fieldCount;
    while ((fieldCount--) >= 0) {
        Field *field = self->fields + fieldCount;
        PixotronVM_free(TO_REF(field->name));
    }
    gint methodCount = (gint) self->methodCount;
    while ((methodCount--) >= 0) {
        Method *method = self->methods + methodCount;
        PixotronVM_free(TO_REF(method->name));
        gint paramCount = method->paramCount;
        while ((paramCount--) >= 0) {
            MethodParam *param = method->params + paramCount;
            PixotronVM_free(TO_REF(param->name));
            PixotronVM_free(TO_REF(param));
        }
    }
    PixotronVM_free(TO_REF(self->fields));
    PixotronVM_free(TO_REF(klass));
}

static Klass *PixtronVM_CreateKlass(const PixtronVM *vm, const gchar *klassName, GFile *file, GError **error) {
    g_return_val_if_fail(klassName != NULL, NULL);
    g_return_val_if_fail(file != NULL, NULL);
    g_return_val_if_fail(error != NULL && *error == NULL, NULL);
    Klass *klass = NULL;
    GFileInfo *fileInfo = NULL;
    GFileInputStream *inputStream = NULL;
    inputStream = g_file_read(file, NULL, error);
    if (*error != NULL) {
        return NULL;
    }
    fileInfo = g_file_query_info(file, G_FILE_ATTRIBUTE_STANDARD_SIZE, G_FILE_QUERY_INFO_NONE, NULL, error);
    if (*error != NULL) {
        g_input_stream_close(G_INPUT_STREAM(inputStream), NULL, NULL);
        g_clear_object(&inputStream);
        return NULL;
    }
    const goffset fileSize = g_file_info_get_size(fileInfo);
    guint8 buf[fileSize];
    g_input_stream_read_all(G_INPUT_STREAM(inputStream), buf, fileSize, NULL, NULL, error);
    g_input_stream_close(G_INPUT_STREAM(inputStream), NULL, NULL);
    g_clear_object(&inputStream);

    if (*error != NULL) {
        goto finally;
    }
    const guint magic = *((guint *) buf);
    if (magic != MAGIC) {
        g_set_error(error, KLASS_DOMAIN, MAGIC_ERROR, "Magic mistake.");
        goto finally;
    }
    klass = PixotronVM_calloc(sizeof(Klass));
    klass->vm = vm;
    klass->magic = magic;
    klass->version = buf[4];
    klass->name = g_strdup(klassName);
    const gint fieldCount = *((gint *) (buf + 5));
    gint position = 9;
    Field *files = PixotronVM_calloc(sizeof(Field) * fieldCount);
    VMValue *values = PixotronVM_calloc(sizeof(VMValue) * fieldCount);
    gint index = 0;
    while (index < fieldCount) {
        Field *field = (files + index);
        field->name = g_strdup((gchar *)(buf + position));
        field->type = *((Type *) (buf + position));
        position = position + PixtronVM_GetStrFullLen(field->name);
        const VMValue value = PixtronVM_CreateValueFromBuffer(field->type, buf + position);
        values[index] = value;
        position = position + TYPE_SIZE[field->type];
        index++;
    }
    klass->fields = files;
    klass->fieldCount = fieldCount;
    klass->methodCount = *((guint *) (buf + position));
    position += 4;
    klass->methods = PixotronVM_calloc(sizeof(Method) * klass->methodCount);
    gint j = 0;
    while (j < klass->methodCount) {
        Method *method = klass->methods + j;
        method->offset = *((guint *) (buf + position));
        position += 4;
        method->ret = buf[position];
        position++;
        const gchar *selfKlass = (gchar *) (buf + position);
        if (g_str_equal(selfKlass, "")) {
            position++;
            method->klass = klass;
        } else {
            position += PixtronVM_GetStrFullLen(klassName);
            Klass *methodKlass = PixtronVM_GetKlass(vm, klassName, error);
            // If other klass load fail exit current klass init
            if (methodKlass == NULL) {
                goto finally;
            }
            method->klass = methodKlass;
        }
        gchar *funcName = g_strdup((gchar *)(buf+position));
        position += PixtronVM_GetStrFullLen(funcName);
        gushort paramCount = *((gushort *) (buf + position));
        position += 2;
        MethodParam *params = NULL;
        if (paramCount > 0) {
            params = PixotronVM_calloc(sizeof(MethodParam) * paramCount);
            for (int i = 0; i < paramCount; ++i) {
                MethodParam *param = params + i;
                params->type = *((Type *) (buf + position));
                position++;
                param->name = g_strdup((gchar *)(buf+position));
                position += PixtronVM_GetStrFullLen(param->name);
            }
        }
        method->name = funcName;
        method->params = params;
        method->paramCount = paramCount;
        j++;
    }
    const glong byteCodeSize = fileSize - position;
    klass->byteCode = PixotronVM_calloc(byteCodeSize);
    memcpy(klass->byteCode, buf + position, byteCodeSize);

finally:
    if (*error != NULL) {
        PixtronVM_FreeKlass(&klass);
    }
    g_clear_object(&fileInfo);
    return klass;
}

static Klass *PixtronVM_KlassLoad(const PixtronVM *vm, const gchar *KlassName, GError **error) {
    GFile *dir = NULL;
    Klass *klass = NULL;
    GFile *KlassFile = NULL;
    GFileInfo *fileInfo = NULL;
    GFileEnumerator *enumerator = NULL;

    dir = g_file_new_for_path(vm->workdir);
    enumerator = g_file_enumerate_children(dir,
                                           G_FILE_ATTRIBUTE_STANDARD_NAME,
                                           G_FILE_QUERY_INFO_NONE,
                                           NULL,
                                           error);
    if (*error != NULL) {
        goto finally;
    }
    while ((fileInfo = g_file_enumerator_next_file(enumerator, NULL, error)) != NULL) {
        const gchar *name = g_file_info_get_name(fileInfo);
        if (!g_str_has_suffix(name, ".clazz")) {
            continue;
        }
        gchar *tmp = g_strndup(name, strlen(name) - 6);
        if (g_str_equal(name, KlassName) == 0) {
            KlassFile = g_file_get_child(dir, name);
            klass = PixtronVM_CreateKlass(vm, tmp, KlassFile, error);
            if (*error != NULL) {
                goto finally;
            }
            g_hash_table_insert(vm->klassTable, tmp, klass);
            break;
        }
        g_free(tmp);
        g_clear_object(&fileInfo);
    }
finally:
    if (*error != NULL) {
        PixtronVM_FreeKlass(&klass);
    }
    g_clear_object(&dir);
    g_clear_object(&KlassFile);
    g_clear_object(&fileInfo);
    g_clear_object(&enumerator);

    return klass;
}

extern inline Klass *PixtronVM_GetKlass(const PixtronVM *vm, const gchar *klassName, GError **error) {
    Klass *klass = g_hash_table_lookup(vm->klassTable, klassName);
    if (klass != NULL) {
        return klass;
    }
    klass = PixtronVM_KlassLoad(vm, klassName, error);
    if (klass == NULL) {
        if (*error != NULL) {
            g_printerr("Find Klass:%s fail:%s", klassName, (*error)->message);
        }
    }
    return klass;
}

extern inline VMValue PixtronVM_GetKlassFileValue(RuntimeContext *context, const guint16 index) {
    const VirtualStackFrame *frame = context->frame;
    const Klass *klass = frame->method->klass;
    const guint count = klass->fieldCount;
    if (index >= count) {
        return NIL;
    }
    const VMValue value = klass->fieldVals[index];
    return value;
}

extern inline Method *PixtronVM_GetKlassMethod(const Klass *klass, const gchar *name) {
    const guint methodCount = klass->methodCount;
    for (int i = 0; i < methodCount; ++i) {
        Method *method = klass->methods + i;
        if (g_str_equal(method->name, name)) {
            return method;
        }
    }
    return NULL;
}
