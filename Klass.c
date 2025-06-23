#include "Klass.h"

#include <stdio.h>
#include <gio/gio.h>

#include "include/engine/Memory.h"
#include "IError.h"
#include "VMString.h"
#include "include/engine/Type.h"
#include "StringUtil.h"

#define MAGIC (0xFFAABBCC)
#define FILE_SUFFIX_LEN (strlen(".klass"))
#define IS_KLASS_FILE(fileName) (g_str_has_suffix(fileName, ".klass"))

static char *PixtronVM_MethodToString(const Method *method) {
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

static inline void PixtronVM_FreeKlass(Klass **klass) {
    if (*klass == NULL) {
        return;
    }
    const Klass *self = *klass;
    PixotronVM_free(TO_REF(self->name));
    PixotronVM_free(TO_REF(self->byteCode));
    int32_t fieldCount = (int32_t) self->fieldCount;
    while ((--fieldCount) >= 0) {
        Field *field = self->fields + fieldCount;
        PixotronVM_free(TO_REF(field->name));
    }
    int32_t methodCount = (int32_t) self->methodCount;
    while ((methodCount--) >= 0) {
        Method *method = self->methods[methodCount];
        if (method == NULL || method->klass != self) {
            continue;
        }
        PixotronVM_free(TO_REF(method->name));
        int16_t argv = (int16_t) method->argv;
        while ((--argv) >= 0) {
            MethodParam *param = method->args + argv;
            PixotronVM_free(TO_REF(param->name));
            PixotronVM_free(TO_REF(param));
        }
    }
    PixotronVM_free(TO_REF(self->fields));
    PixotronVM_free(CAST_REF(klass));
}

static inline int32_t PixtronVM_BuilderConst(const PixtronVM *vm, Klass *klass, const uint16_t constSize,
                                             const uint8_t *buf,
                                             int32_t position) {
    VMValue **constArr = PixotronVM_calloc(sizeof(VMValue *) * constSize);
    for (uint16_t i = 0; i < constSize; i++) {
        const Type type = *(Type *) (buf + position);
        position += 2;
        if (type != TYPE_STRING) {
            g_warning("Constant pool only support current type is '%s' and it will be ignored.\n", TYPE_NAME[type]);
            position += TYPE_SIZE[type];
            continue;
        }
        const int32_t length = *((int32_t *) (buf + position));
        position += 4;
        String string = {
            length,
            (char *) (buf + position)
        };
        String *ptr = PixtronVM_StringCheckPool(vm, &string);
        VMValue *constVal = PixotronVM_calloc(sizeof(VMValue));
        constVal->type = TYPE_STRING;
        constVal->obj = ptr;
        constArr[i] = constVal;
        position += length;
    }
    klass->constArray = constArr;

    return position;
}

static inline Klass *PixtronVM_CreateKlass(const PixtronVM *vm, const char *klassName, GFile *file, GError **error) {
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
    uint8_t buf[fileSize];
    g_input_stream_read_all(G_INPUT_STREAM(inputStream), buf, fileSize, NULL, NULL, error);
    g_input_stream_close(G_INPUT_STREAM(inputStream), NULL, NULL);
    g_clear_object(&inputStream);

    if (*error != NULL) {
        goto finally;
    }
    const int32_t magic = *((int32_t *) buf);
    if (magic != MAGIC) {
        g_set_error(error, KLASS_DOMAIN, MAGIC_ERROR, "Magic mistake.");
        goto finally;
    }
    klass = PixotronVM_calloc(sizeof(Klass));
    klass->vm = vm;
    klass->magic = magic;
    int32_t position = 4;
    klass->version = *((Version *) (buf + position));
    position += 2;
    const uint16_t constSize = *((uint16_t *) (buf + position));
    position += 2;
    if (constSize > 0) {
        position = PixtronVM_BuilderConst(vm, klass, constSize, buf, position);
    }
    klass->constSize = constSize;
    klass->name = g_strdup(klassName);
    klass->fieldCount = *((int32_t *) (buf + position));
    position += 4;
    Field *files = PixotronVM_calloc(sizeof(Field) * klass->fieldCount);
    VMValue *values = PixotronVM_calloc(VM_VALUE_SIZE * klass->fieldCount);
    uint32_t index = 0;
    while (index < klass->fieldCount) {
        Field *field = (files + index);
        const Type type = *((Type *) (buf + position));
        position += 2;
        char *name = g_strdup((gchar *)(buf + position));
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
    klass->methods = PixotronVM_calloc(sizeof(Method *) * klass->methodCount);
    uint32_t j = 0;
    while (j < klass->methodCount) {
        const char *selfKlassName = (char *) (buf + position);
        position += PixtronVM_GetStrFullLen(selfKlassName);
        const char *funcName = (char *) (buf + position);
        position += PixtronVM_GetStrFullLen(funcName);
        if (!g_str_equal(selfKlassName, "")) {
            const Klass *selfKlass = PixtronVM_GetKlass(vm, selfKlassName, error);
            if (selfKlass == NULL) {
                goto finally;
            }
            Method *method = PixtronVM_GetKlassMethodByName(selfKlass, funcName);
            if (method == NULL) {
                g_set_error(error, KLASS_DOMAIN, METHOD_NOT_FOUND, "Method %s not found in class %s", funcName,
                            selfKlassName);
                goto finally;
            }
            klass->methods[j] = method;
        } else {
            Method *method = PixotronVM_calloc(sizeof(Method));
            method->klass = klass;
            method->nativeFunc = *((bool *) (buf + position));
            position++;
            if (method->nativeFunc) {
                const char *libs = (char *) (buf + position);
                if (g_str_equal(libs, "")) {
                    method->libCount = 0;
                } else {
                    char **arr = g_strsplit(libs, "|", 1024);
                    method->libName = arr;
                    method->libCount = g_strv_length(arr);
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
            const uint16_t argv = *((uint16_t *) (buf + position));
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
            method->argv = argv;
            method->args = args;
            klass->methods[j] = method;
            method->name = g_strdup(funcName);
            method->toString = PixtronVM_MethodToString;
            // Native method automatic calculate stack size
            if (method->nativeFunc) {
                method->maxLocalsSize = argv;
                method->maxStackSize = method->retType != TYPE_VOID;
            }
        }
        j++;
    }
    const uint64_t byteCodeSize = fileSize - position;
    if (byteCodeSize > 0) {
        klass->byteCode = PixotronVM_calloc(byteCodeSize);
        memcpy(klass->byteCode, buf + position, byteCodeSize);
    }
finally:
    if (*error != NULL) {
        PixtronVM_FreeKlass(&klass);
    }
    g_clear_object(&fileInfo);
    return klass;
}

static Klass *PixtronVM_KlassLoad(const PixtronVM *vm, const char *klassName, GError **error) {
    GFile *dir = NULL;
    Klass *klass = NULL;
    GFile *klassFile = NULL;
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
        const char *fileName = g_file_info_get_name(fileInfo);
        if (!IS_KLASS_FILE(fileName)) {
            continue;
        }
        const size_t strLen = strlen(fileName) - 6;
        if (strncmp(klassName, fileName, strLen) == 0) {
            klassFile = g_file_get_child(dir, fileName);
            klass = PixtronVM_CreateKlass(vm, klassName, klassFile, error);
            if (*error != NULL) {
                goto finally;
            }
            break;
        }
        g_clear_object(&fileInfo);
    }
    if (klassFile == NULL) {
        g_set_error(error, KLASS_DOMAIN, KLASS_NOT_FOUND, "%s.klass not found.\n", klassName);
    }
finally:
    g_clear_object(&dir);
    g_clear_object(&klassFile);
    g_clear_object(&fileInfo);
    g_clear_object(&enumerator);

    return klass;
}

extern inline Klass *PixtronVM_GetKlass(const PixtronVM *vm, const char *klassName, GError **error) {
    Klass *klass = g_hash_table_lookup(vm->klassTable, klassName);
    if (klass != NULL) {
        return klass;
    }
    // If Klass path is null skip klass load
    if (vm->klassPath == NULL) {
        return NULL;
    }
    klass = PixtronVM_KlassLoad(vm, klassName, error);
    if (klass != NULL) {
        g_hash_table_insert(vm->klassTable, klass->name, klass);
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

extern inline void PixtronVM_GetKlassConstant(RuntimeContext *context, const uint16_t index, VMValue *value) {
    const Klass *klass = context->frame->method->klass;
    const uint16_t constSize = klass->constSize;
    if (index >= constSize) {
        context->throwException(context, "Constant pool index out of bounds: requested #%d (valid range: 0-%d)", index,
                                constSize);
    }
    memcpy(value, klass->constArray[index], VM_VALUE_SIZE);
}

extern inline Method *PixtronVM_GetKlassMethod(RuntimeContext *context, const uint16_t index) {
    const Klass *klass = context->frame->method->klass;
    const uint32_t methodCount = klass->methodCount;
    if (index >= methodCount) {
        context->throwException(context, "Method index out of range current index is [%d] but max index is [%d].",
                                index, methodCount);
        return NULL;
    }
    return klass->methods[index];
}

extern inline Method *PixtronVM_GetKlassMethodByName(const Klass *klass, const char *name) {
    const uint32_t methodCount = klass->methodCount;
    for (int i = 0; i < methodCount; ++i) {
        Method *method = klass->methods[i];
        if (g_str_equal(method->name, name)) {
            return method;
        }
    }
    return NULL;
}

extern inline void PixtronVM_InitSystemKlass(const PixtronVM *vm, GError **error) {
    const char *klassPath = g_hash_table_lookup(vm->envs, "KlassPath");
    if (klassPath == NULL) {
        return;
    }
    GFile *klassPathDir = NULL;
    GFileInfo *fileInfo = NULL;
    GFileEnumerator *enumerator = NULL;
    klassPathDir = g_file_new_for_path(klassPath);
    enumerator = g_file_enumerate_children(klassPathDir,
                                           G_FILE_ATTRIBUTE_STANDARD_NAME,
                                           G_FILE_QUERY_INFO_NONE,
                                           NULL,
                                           error);
    if (*error != NULL) {
        g_clear_object(&klassPathDir);
        return;
    }

    for (;;) {
        if (fileInfo != NULL) {
            g_clear_object(&fileInfo);
        }
        fileInfo = g_file_enumerator_next_file(enumerator, NULL, error);
        if (fileInfo == NULL) {
            break;
        }
        const char *name = g_file_info_get_name(fileInfo);
        if (!IS_KLASS_FILE(name)) {
            continue;
        }
        GFile *klassFile = g_file_get_child(klassPathDir, name);
        const size_t t = strlen(name) - FILE_SUFFIX_LEN + 1;
        char klassName[t];
        snprintf(klassName, t, "%s", name);
        const bool exist = g_hash_table_contains(vm->klassTable, klassName);
        if (exist) {
            g_clear_object(&klassFile);
            continue;
        }
        Klass *klass = PixtronVM_CreateKlass(vm, klassName, klassFile, error);
        g_clear_object(&klassFile);
        if (klass == NULL) {
            goto finally;
        }
        g_hash_table_insert(vm->klassTable, klass->name, klass);
    }
finally:
    g_clear_object(&enumerator);
    g_clear_object(&klassPathDir);
}
