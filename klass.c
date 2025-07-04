#include "klass.h"

#include <stdio.h>
#include <gio/gio.h>

#include "memory.h"
#include "ierror.h"
#include "itype.h"
#include "helper.h"
#include "istring.h"
#include "config.h"

#define MAGIC (0xFFAABBCC)
#define FILE_SUFFIX_LEN (strlen(".klass"))
#define IS_KLASS_FILE(fileName) (g_str_has_suffix(fileName, ".klass"))

static inline char *PixtronVM_MethodToString(const Method *method) {
    GString *str = g_string_new(NULL);
    const Type retType = method->ret;
    const char *retTypeName;
    if (retType == NIL) {
        retTypeName = "void";
    } else {
        retTypeName = TYPE_NAME[retType];
    }
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
    g_string_append(str, " -> ");
    g_string_append(str, retTypeName);
    return g_string_free(str, FALSE);
}

static inline void PixtronVM_FreeKlass(Klass **klass) {
    if (*klass == NULL) {
        return;
    }
    const Klass *self = *klass;
    pvm_mem_free(TO_REF(self->name));
    pvm_mem_free(TO_REF(self->bytecode));
    int32_t fieldCount = (int32_t) self->fieldCount;
    while ((--fieldCount) >= 0) {
        Field *field = self->fields + fieldCount;
        pvm_mem_free(TO_REF(field->name));
    }
    int32_t methodCount = (int32_t) self->method_count;
    while ((--methodCount) >= 0) {
        Method *method = self->methods[methodCount];
        if (method == NULL || method->klass != self) {
            continue;
        }
        pvm_mem_free(TO_REF(method->name));
        int16_t argv = (int16_t) method->argv;
        while ((--argv) >= 0) {
            MethodParam *param = method->args + argv;
            pvm_mem_free(TO_REF(param->name));
            pvm_mem_free(TO_REF(param));
        }
    }
    pvm_mem_free(TO_REF(self->fields));
    pvm_mem_free(CAST_REF(klass));
}

static inline int32_t pvm_build_constant(const PixtronVM *vm, Klass *klass, const uint16_t constSize,
                                         const uint8_t *buf,
                                         int32_t position) {
    VMValue **constants = pvm_mem_calloc(sizeof(VMValue *) * constSize);
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
        String *ptr = pvm_string_const_pool_add(vm, &string);
        VMValue *constVal = pvm_mem_calloc(sizeof(VMValue));
        constVal->type = TYPE_STRING;
        constVal->obj = ptr;
        constants[i] = constVal;
        position += length;
    }
    klass->constants = constants;

    return position;
}

static inline Klass *pvm_create_klass(const PixtronVM *vm, const char *klassName, GFile *file, GError **error) {
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
    klass = pvm_mem_calloc(sizeof(Klass));
    klass->vm = vm;
    klass->magic = magic;
    int32_t position = 4;
    klass->version = *((Version *) (buf + position));
    position += 2;
    const uint8_t libraryFlag = *((uint8_t *) (buf + position));
    position++;
    if (libraryFlag) {
        char *library = g_strdup((char *)(buf+position));
        klass->library = library;
        position += pvm_get_cstr_len(library);
    }
    const uint16_t const_size = *((uint16_t *) (buf + position));
    position += 2;
    if (const_size > 0) {
        position = pvm_build_constant(vm, klass, const_size, buf, position);
    }
    klass->const_size = const_size;
    klass->name = g_strdup(klassName);
    klass->fieldCount = *((uint32_t *) (buf + position));
    position += 4;
    Field *files = pvm_mem_calloc(sizeof(Field) * klass->fieldCount);
    uint32_t index = 0;
    while (index < klass->fieldCount) {
        Field *field = (files + index);
        const Type type = *((Type *) (buf + position));
        position += 2;
        char *name = g_strdup((gchar *)(buf + position));
        position += pvm_get_cstr_len(name);
        VMValue *value = pvm_mem_calloc(VM_VALUE_SIZE);
        position += pvm_load_typed_value_from_buffer(type, value, buf);
        value->type = type;
        field->name = name;
        field->value = value;
        index++;
    }
    klass->fields = files;
    klass->method_count = *((uint32_t *) (buf + position));
    position += 4;
    klass->methods = pvm_mem_calloc(sizeof(Method *) * klass->method_count);
    uint32_t j = 0;
    while (j < klass->method_count) {
        const char *selfKlassName = (char *) (buf + position);
        position += pvm_get_cstr_len(selfKlassName);
        const char *funcName = (char *) (buf + position);
        position += pvm_get_cstr_len(funcName);
        if (!g_str_equal(selfKlassName, "")) {
            const Klass *self_klass = pvm_get_klass(vm, selfKlassName, error);
            if (self_klass == NULL) {
                goto finally;
            }
            Method *method = pvm_get_method_by_name(self_klass, funcName);
            if (method == NULL) {
                g_set_error(error, KLASS_DOMAIN, METHOD_NOT_FOUND, "Method %s not found in class %s", funcName,
                            selfKlassName);
                goto finally;
            }
            klass->methods[j] = method;
        } else {
            Method *method = pvm_mem_calloc(sizeof(Method));
            method->klass = klass;
            method->native_func = *((uint8_t *) (buf + position)) == 1;
            position++;
            if (method->native_func) {
                const char *library = (char *) (buf + position);
                if (!g_str_equal(library, "")) {
                    method->lib_name = g_strdup(library);
                }
                position += pvm_get_cstr_len(library);
            }
            method->max_locals = *((uint16_t *) (buf + position));
            position += 2;
            method->max_stacks = *((uint16_t *) (buf + position));
            position += 2;
            method->offset = *((uint32_t *) (buf + position));
            position += 4;
            method->end_offset = method->offset + (*((uint32_t *) (buf + position)));
            position += 4;
            method->ret = *((Type *) (buf + position));
            position += 2;
            const uint16_t argv = *((uint16_t *) (buf + position));
            position += 2;
            MethodParam *args = NULL;
            if (argv > 0) {
                args = pvm_mem_calloc(sizeof(MethodParam) * argv);
                for (int i = 0; i < argv; ++i) {
                    MethodParam *param = args + i;
                    param->type = *((Type *) (buf + position));
                    position += 2;
                    param->name = g_strdup((gchar *)(buf+position));
                    position += pvm_get_cstr_len(param->name);
                }
            }
            method->argv = argv;
            method->args = args;
            klass->methods[j] = method;
            method->name = g_strdup(funcName);
            method->toString = PixtronVM_MethodToString;
            // Native method automatic calculate stack size
            if (method->native_func) {
                const bool success = pvm_lookup_native_handle(klass, method, error);
                if (!success) {
                    goto finally;
                }
                method->max_locals = argv;
                method->max_stacks = method->ret != TYPE_VOID;
            }
        }
        j++;
    }
    const uint64_t byteCodeSize = fileSize - position;
    if (byteCodeSize > 0) {
        klass->bytecode = pvm_mem_calloc(byteCodeSize);
        memcpy(klass->bytecode, buf + position, byteCodeSize);
    }
finally:
    if (*error != NULL) {
        PixtronVM_FreeKlass(&klass);
    }
    g_clear_object(&fileInfo);
    return klass;
}

static inline Klass *pvm_load_klass(const PixtronVM *vm, const char *klassName, GError **error) {
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
            klass = pvm_create_klass(vm, klassName, klassFile, error);
            if (*error != NULL) {
                goto finally;
            }
            break;
        }
        g_clear_object(&fileInfo);
    }
    if (klassFile == NULL) {
        g_set_error(error, KLASS_DOMAIN, KLASS_NOT_FOUND, "%s.klass not found.", klassName);
    }
finally:
    g_clear_object(&dir);
    g_clear_object(&klassFile);
    g_clear_object(&fileInfo);
    g_clear_object(&enumerator);

    return klass;
}

extern inline Klass *pvm_get_klass(const PixtronVM *vm, const char *klassName, GError **error) {
    Klass *klass = g_hash_table_lookup(vm->klasses, klassName);
    if (klass != NULL) {
        return klass;
    }
    // If Klass path is null skip klass load
    if (vm->klassPath == NULL) {
        return NULL;
    }
    klass = pvm_load_klass(vm, klassName, error);
    if (klass != NULL) {
        g_hash_table_insert(vm->klasses, klass->name, klass);
    }
    return klass;
}

static inline const Klass *pvm_check_field_index(RuntimeContext *context, const guint16 index) {
    const VirtualStackFrame *frame = context->frame;
    const Klass *klass = frame->method->klass;
    const guint count = klass->fieldCount;
    if (index >= count) {
        context->throw_exception(context, "Klass index out of bounds.");
    }
    return klass;
}

extern inline void pvm_get_klass_field(RuntimeContext *context, const uint16_t index, VMValue *value) {
    const Klass *klass = pvm_check_field_index(context, index);
    const Field *field = klass->fields + index;
    memcpy(value, field->value, VM_VALUE_SIZE);
}


extern inline void pvm_set_klass_field(RuntimeContext *context, const guint16 index, const VMValue *value) {
    const Klass *klass = pvm_check_field_index(context, index);
    const Field *field = klass->fields + index;
    const VMValue *_value = field->value;
#if VM_DEBUG_ENABLE
    // Type check
    if (_value->type != value->type) {
        context->throw_exception(context, "Klass field type is:%d but value type is:%d.", _value->type, value->type);
    }
#endif
    memcpy(field->value, value, VM_VALUE_SIZE);
}

extern inline void pvm_get_klass_constant(RuntimeContext *context, const uint16_t index, VMValue *value) {
    const Klass *klass = context->frame->method->klass;
    const uint16_t constSize = klass->const_size;
    if (index >= constSize) {
        context->throw_exception(context, "Constant pool index out of bounds: requested #%d (valid range: 0-%d)", index,
                                 constSize);
    }
    memcpy(value, klass->constants[index], VM_VALUE_SIZE);
}

extern inline Method *pvm_get_method(RuntimeContext *context, const uint16_t index) {
    const Klass *klass = context->frame->method->klass;
    const uint32_t method_count = klass->method_count;
    if (index >= method_count) {
        context->throw_exception(context, "Method index out of range current index is [%d] but max index is [%d].",
                                 index, method_count);
        return NULL;
    }
    return klass->methods[index];
}

extern inline Method *pvm_get_method_by_name(const Klass *klass, const char *name) {
    const uint32_t method_count = klass->method_count;
    for (int i = 0; i < method_count; ++i) {
        Method *method = klass->methods[i];
        if (g_str_equal(method->name, name)) {
            return method;
        }
    }
    return NULL;
}

extern inline void pvm_load_system_klass(const PixtronVM *vm, GError **error) {
    const char *klass_path = g_hash_table_lookup(vm->envs, "KlassPath");
    if (klass_path == NULL) {
        return;
    }
    GFile *klass_path_dir = NULL;
    GFileInfo *file_info = NULL;
    GFileEnumerator *enumerator = NULL;
    klass_path_dir = g_file_new_for_path(klass_path);
    enumerator = g_file_enumerate_children(klass_path_dir,
                                           G_FILE_ATTRIBUTE_STANDARD_NAME,
                                           G_FILE_QUERY_INFO_NONE,
                                           NULL,
                                           error);
    if (*error != NULL) {
        g_clear_object(&klass_path_dir);
        return;
    }

    for (;;) {
        if (file_info != NULL) {
            g_clear_object(&file_info);
        }
        file_info = g_file_enumerator_next_file(enumerator, NULL, error);
        if (file_info == NULL) {
            break;
        }
        const char *name = g_file_info_get_name(file_info);
        if (!IS_KLASS_FILE(name)) {
            continue;
        }
        GFile *klassFile = g_file_get_child(klass_path_dir, name);
        const size_t t = strlen(name) - FILE_SUFFIX_LEN + 1;
        char klass_name[t];
        snprintf(klass_name, t, "%s", name);
        const bool exist = g_hash_table_contains(vm->klasses, klass_name);
        if (exist) {
            g_clear_object(&klassFile);
            continue;
        }
        Klass *klass = pvm_create_klass(vm, klass_name, klassFile, error);
        g_clear_object(&klassFile);
        if (klass == NULL) {
            goto finally;
        }
        g_hash_table_insert(vm->klasses, klass->name, klass);
    }
finally:
    g_clear_object(&enumerator);
    g_clear_object(&klass_path_dir);
}
