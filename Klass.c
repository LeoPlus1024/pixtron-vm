#include "Klass.h"

#include <stdio.h>
#include <gio/gio.h>

#include "Memory.h"
#include "IError.h"
#include "Type.h"
#include "StringUtil.h"

#define MAGIC (0xFFAABBCC)

static gchar *PixtronVM_MethodToString(const Method *method) {
    GString *str = g_string_new(NULL);
    const Type retType = method->retType;
    const char *retTypeName;
    if (retType == NIL) {
        retTypeName = "void";
    } else {
        retTypeName = TYPE_NAME[retType];
    }
    g_string_append(str, retTypeName);
    g_string_append(str, " ");
    g_string_append(str, method->name);
    g_string_append_c(str, '(');
    const uint16_t argv = method->argv;
    for (gushort i = 0; i < argv; i++) {
        const MethodParam *param = method->args + i;
        const char *typeName = TYPE_NAME[param->type];
        g_string_append(str, typeName);
        g_string_append(str, " ");
        g_string_append(str, param->name);
        if (i != argv - 1) {
            g_string_append_c(str, ',');
        }
    }
    g_string_append_c(str, ')');
    return g_string_free(str, FALSE);
}

static void PixtronVM_FreeKlass(Klass **klass) {
    if (*klass == NULL) {
        return;
    }
    const Klass *self = *klass;
    PixotronVM_free(TO_REF(self->name));
    PixotronVM_free(TO_REF(self->byteCode));
    int32_t fieldCount = (int32_t) self->fieldCount;
    while ((fieldCount--) >= 0) {
        Field *field = self->fields + fieldCount;
        PixotronVM_free(TO_REF(field->name));
    }
    int32_t methodCount = (int32_t) self->methodCount;
    while ((methodCount--) >= 0) {
        Method *method = self->methods + methodCount;
        PixotronVM_free(TO_REF(method->name));
        int16_t argv = (int16_t) method->argv;
        while ((argv--) >= 0) {
            MethodParam *param = method->args + argv;
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
    gint position = 4;
    klass->vm = vm;
    klass->magic = magic;
    klass->version = *((Version *) (buf + position));
    position += 2;
    klass->name = g_strdup(klassName);
    klass->fieldCount = *((gint *) (buf + position));
    position += 4;
    Field *files = PixotronVM_calloc(sizeof(Field) * klass->fieldCount);
    VMValue *values = PixotronVM_calloc(VM_VALUE_SIZE * klass->fieldCount);
    uint32_t index = 0;
    while (index < klass->fieldCount) {
        Field *field = (files + index);
        const Type type = *((Type *) (buf + position));
        position += 2;
        gchar *name = g_strdup((gchar *)(buf + position));
        position += PixtronVM_GetStrFullLen(name);
        VMValue *value = values + index;
        const uint8_t typeSize = TYPE_SIZE[type];
        memcpy(value, buf, typeSize);
        value->type = type;
        position = position + typeSize;
        field->name = name;
        index++;
    }
    klass->fields = files;
    klass->methodCount = *((uint32_t *) (buf + position));
    position += 4;
    klass->methods = PixotronVM_calloc(sizeof(Method) * klass->methodCount);
    uint32_t j = 0;
    while (j < klass->methodCount) {
        Method *method = klass->methods + j;
        method->nativeFunc = *((uint8_t *) (buf + position));
        position++;
        if (method->nativeFunc) {
            const char *libs = (char *) (buf + position);
            if (g_str_equal(libs, "")) {
                method->libCount = 0;
            } else {
                char **arr = g_strsplit(libs, "|", 1024);
                uint16_t libCount = 0;
                while (arr[libCount] != NULL) {
                    libCount++;
                }
                method->libName = arr;
                method->libCount = libCount;
                position += PixtronVM_GetStrFullLen(libs);
            }
        }
        method->maxLocalsSize = *((uint16_t *) (buf + position));
        position += 2;
        method->maxStackSize = *((uint16_t *) (buf + position));
        position += 2;
        method->offset = *((uint32_t *) (buf + position));
        position += 4;
        method->endOffset = *((uint32_t *) (buf + position));
        position += 4;
        method->retType = *((Type *) (buf + position));
        position += 2;
        const char *selfKlass = (char *) (buf + position);
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
        const uint16_t argv = *((gushort *) (buf + position));
        position += 2;
        MethodParam *args = NULL;
        if (argv > 0) {
            args = PixotronVM_calloc(sizeof(MethodParam) * argv);
            for (int i = 0; i < argv; ++i) {
                MethodParam *param = args + i;
                param->type = *((Type *) (buf + position));
                position += 2;
                param->name = g_strdup((gchar *)(buf+position));
                position += PixtronVM_GetStrFullLen(param->name);
            }
        }
        method->name = funcName;
        method->args = args;
        method->toString = PixtronVM_MethodToString;
        method->argv = argv;
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

    dir = g_file_new_for_path(vm->klassPath);
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
        if (!g_str_has_suffix(name, ".klass")) {
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

static inline const Klass *PixtronVM_KlassFieldOutOfBoundsCheck(RuntimeContext *context, const guint16 index) {
    const VirtualStackFrame *frame = context->frame;
    const Klass *klass = frame->method->klass;
    const guint count = klass->fieldCount;
    if (index >= count) {
        context->throwException(context, "Klass index out of bounds.");
    }
    return klass;
}

extern inline void PixtronVM_GetKlassFileValue(RuntimeContext *context, const uint16_t index, VMValue *value) {
    const Klass *klass = PixtronVM_KlassFieldOutOfBoundsCheck(context, index);
    const Field *field = klass->fields + index;
    memcpy(value, field->value, VM_VALUE_SIZE);
}


extern inline void PixtronVM_SetKlassFileValue(RuntimeContext *context, const guint16 index, const VMValue *value) {
    const Klass *klass = PixtronVM_KlassFieldOutOfBoundsCheck(context, index);
    const Field *field = klass->fields + index;
    const VMValue *_value = field->value;
    // Type check
    if (_value->type != value->type) {
        context->throwException(context, "Klass field type is:%d but value type is:%d.", _value->type, value->type);
    }
    memcpy(field->value, value, VM_VALUE_SIZE);
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
