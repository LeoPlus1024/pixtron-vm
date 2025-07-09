#include "pklass.h"

#include <stdio.h>
#include <gio/gio.h>

#include "pmem.h"
#include "perr.h"
#include "phelper.h"
#include "pstr.h"
#include "config.h"
#include "pcore.h"
#include "pffi.h"

#define MAGIC (0xFFAABBCC)
#define FILE_SUFFIX_LEN (strlen(".klass"))
#define IS_KLASS_FILE(fileName) (g_str_has_suffix(fileName, ".klass"))

static inline void pvm_exec_klass_constructor(const Klass *klass) {
    if (klass == NULL || klass->constructor == NULL) {
        return;
    }
    const Method *method = klass->constructor;
    const CallMethodParam param = {
        method,
        0,
        NULL
    };
    pvm_call_method(&param);
}

static inline void pvm_load_symbol0(const Klass *klass, const uint8_t *buffer, int32_t *pos, Symbol *symbol) {
    const uint16_t index = *((uint16_t *) (buffer + (*pos)));
    PStr *pstr = klass->constants[index];
    symbol->name = pstr;
    symbol->index = index;
    *pos += 2;
}


static inline Symbol *pvm_load_symbol(const Klass *klass, const uint8_t *buffer, int32_t *pos) {
    Symbol *symbol = pvm_mem_calloc(sizeof(Symbol));
    pvm_load_symbol0(klass, buffer, pos, symbol);
    return symbol;
}


static inline char *pvm_method_tostring(const Method *method) {
    GString *str = g_string_new(NULL);
    const Type retType = method->ret;
    const char *retTypeName;
    if (retType == NIL) {
        retTypeName = "void";
    } else {
        retTypeName = TYPE_NAME[retType];
    }
    g_string_append(str, pvm_get_symbol_value(method->name));
    g_string_append_c(str, '(');
    const uint16_t argv = method->argc;
    for (gushort i = 0; i < argv; i++) {
        const MethodParam *param = method->argv + i;
        const char *typeName = TYPE_NAME[param->type];
        g_string_append(str, typeName);
        g_string_append(str, " ");
        g_string_append(str, pvm_get_symbol_value(param->name));
        if (i != argv - 1) {
            g_string_append_c(str, ',');
        }
    }
    g_string_append_c(str, ')');
    g_string_append(str, ":");
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
    int32_t fieldCount = (int32_t) self->fcount;
    while ((--fieldCount) >= 0) {
        Field *field = self->fields + fieldCount;
        pvm_mem_free(TO_REF(field->name));
    }
    int32_t methodCount = (int32_t) self->mcount;
    while ((--methodCount) >= 0) {
        Method *method = self->methods[methodCount];
        if (method == NULL || method->klass != self) {
            continue;
        }
        pvm_mem_free(TO_REF(method->name));
        int16_t argc = (int16_t) method->argv;
        while ((--argc) >= 0) {
            MethodParam *param = method->argv + argc;
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
    PStr **constants = pvm_mem_calloc(sizeof(PStr *) * constSize);
    for (uint16_t i = 0; i < constSize; i++) {
        const int32_t length = *((int32_t *) (buf + position));
        position += 4;
        char *c_str = (char *) buf + position;
        PStr *ptr = pvm_string_intern(vm, length, c_str);
        constants[i] = ptr;
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
    const int64_t fileSize = g_file_info_get_size(fileInfo);
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
    const uint16_t const_size = *((uint16_t *) (buf + position));
    position += 2;
    if (const_size > 0) {
        position = pvm_build_constant(vm, klass, const_size, buf, position);
    }
    // Namespace
    klass->name = pvm_load_symbol(klass, buf, &position);
    // Source file name
    klass->file = pvm_load_symbol(klass, buf, &position);
    const uint8_t lib_flag = *(buf + position++);
    if (lib_flag) {
        klass->library = pvm_load_symbol(klass, buf, &position);
    } else {
        klass->library = NULL;
    }
    klass->const_size = const_size;
    klass->fcount = *((uint32_t *) (buf + position));
    position += 4;
    Field *files = pvm_mem_calloc(sizeof(Field) * klass->fcount);
    uint32_t index = 0;
    while (index < klass->fcount) {
        Field *field = (files + index);
        const Type type = *((Type *) (buf + position));
        position += 2;
        VMValue *value = pvm_mem_calloc(VM_VALUE_SIZE);
        value->type = type;
        field->name = pvm_load_symbol(klass, buf, &position);
        field->value = value;
        index++;
    }
    klass->fields = files;
    klass->mcount = *((uint32_t *) (buf + position));
    position += 4;
    klass->methods = pvm_mem_calloc(sizeof(Method *) * klass->mcount);
    uint32_t j = 0;
    while (j < klass->mcount) {
        const Symbol *ksymbol = pvm_load_symbol(klass, buf, &position);
        Symbol *fsymbol = pvm_load_symbol(klass, buf, &position);
        Method *method = NULL;
        if (!pvm_symbol_equal(ksymbol, klass->name)) {
            const char *self_klass_name = pvm_get_symbol_value(ksymbol);
            const Klass *self_klass = pvm_get_klass(vm, self_klass_name, error);
            if (self_klass == NULL) {
                goto finally;
            }
            char *func_name = pvm_get_symbol_value(fsymbol);
            method = pvm_get_method_by_name(self_klass, func_name);
            if (method == NULL) {
                g_set_error(error, KLASS_DOMAIN, METHOD_NOT_FOUND, "Method %s not found in class %s", func_name,
                            self_klass_name);
                goto finally;
            }
        } else {
            method = pvm_mem_calloc(sizeof(Method));
            method->klass = klass;
            method->native_func = *(buf + position++) == 1;
            int16_t n_attr = *((int16_t *) (buf + position));
            position += 2;
            while (n_attr-- >= 1) {
                Symbol attr;
                Symbol value;
                Symbol *attr_ptr = &attr;
                Symbol *value_ptr = &value;
                pvm_load_symbol0(klass, buf, &position, attr_ptr);
                pvm_load_symbol0(klass, buf, &position, value_ptr);
                if (pvm_symbol_cmp_cstr(attr_ptr, PFFI_DESTROY)) {
                    method->cleanup = pvm_symbol_dup(attr_ptr);
                } else if (pvm_symbol_cmp_cstr(attr_ptr, PFFI_RAW_STR)) {
                    method->raw_str = pvm_symbol_cmp_cstr(value_ptr,PFFI_ON);
                }
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
            const uint16_t argc = *((uint16_t *) (buf + position));
            position += 2;
            MethodParam *argv = NULL;
            if (argc > 0) {
                argv = pvm_mem_calloc(sizeof(MethodParam) * argc);
                for (int i = 0; i < argc; ++i) {
                    MethodParam *param = argv + i;
                    param->type = *((Type *) (buf + position));
                    position += 2;
                    param->name = pvm_load_symbol(klass, buf, &position);
                }
            }
            method->argc = argc;
            method->argv = argv;
            method->name = fsymbol;
            method->toString = pvm_method_tostring;
            // Native method automatic calculate stack size
            if (method->native_func) {
                const bool success = pvm_lookup_native_handle(klass, method, error);
                if (!success) {
                    goto finally;
                }
                method->max_locals = argc;
                method->max_stacks = method->ret != TYPE_VOID;
            }
        }
        if (pvm_symbol_cmp_cstr(method->name, "<cinit>")) {
            if (klass->constructor != NULL) {
                g_set_error(error, KLASS_DOMAIN, CONSTRUCTOR_REPEATED, "Multiple constructors in class %s", klassName);
                goto finally;
            }
            klass->constructor = method;
        }
        klass->methods[j] = method;
        j++;
    }
    uint32_t mcount = klass->mcount;
    while (mcount-- > 0) {
        Method *method = klass->methods[mcount];
        if (method->m_cleanup != NULL) {
            const char *destroy_name = pvm_get_symbol_value(method->cleanup);
            Method *cleanup = pvm_get_method_by_name(klass, destroy_name);
            if (cleanup == NULL) {
                g_set_error(error, KLASS_DOMAIN, METHOD_NOT_FOUND, "Cleanup method <%s> not found in method <%s>",
                            destroy_name, pvm_get_symbol_value(method->name));
                goto finally;
            }
            if (cleanup->argc == 1 && cleanup->argv[0].type == TYPE_HANDLE) {
                method->m_cleanup = cleanup;
            } else {
                g_set_error(error, KLASS_DOMAIN, METHOD_SIGNATURE_ERROR,
                            "Cleanup method must be a function taking one argument of type `handle` and returning void"
                );
            }
        }
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

static inline Klass *pvm_load_klass(const PixtronVM *vm, const char *klass_name, GError **error) {
    GFile *dir = NULL;
    Klass *klass = NULL;
    GFile *klas_file = NULL;
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
        if (strncmp(klass_name, fileName, strLen) == 0) {
            klas_file = g_file_get_child(dir, fileName);
            klass = pvm_create_klass(vm, klass_name, klas_file, error);
            if (*error != NULL) {
                goto finally;
            }
            break;
        }
        g_clear_object(&fileInfo);
    }
    if (klas_file == NULL) {
        g_set_error(error, KLASS_DOMAIN, KLASS_NOT_FOUND, "%s.klass not found.", klass_name);
    }
finally:
    g_clear_object(&dir);
    g_clear_object(&klas_file);
    g_clear_object(&fileInfo);
    g_clear_object(&enumerator);

    return klass;
}

extern inline Klass *pvm_get_klass(const PixtronVM *vm, const char *klass_name, GError **error) {
    Klass *klass = g_hash_table_lookup(vm->klasses, klass_name);
    if (klass != NULL) {
        return klass;
    }
    // If Klass path is null skip klass load
    if (vm->klassPath == NULL) {
        return NULL;
    }
    klass = pvm_load_klass(vm, klass_name, error);
    if (klass != NULL) {
        pvm_exec_klass_constructor(klass);
        g_hash_table_insert(vm->klasses, klass->name, klass);
    }
    return klass;
}

static inline const Klass *pvm_check_field_index(RuntimeContext *context, const guint16 index) {
    const VirtualStackFrame *frame = context->frame;
    const Klass *klass = frame->method->klass;
    const guint count = klass->fcount;
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
    value->type = TYPE_STRING;
    value->obj = klass->constants[index];
}

extern inline Method *pvm_get_method(RuntimeContext *context, const uint16_t index) {
    const Klass *klass = context->frame->method->klass;
    const uint32_t method_count = klass->mcount;
    if (index >= method_count) {
        context->throw_exception(context, "Method index out of range current index is [%d] but max index is [%d].",
                                 index, method_count);
        return NULL;
    }
    return klass->methods[index];
}

extern inline Method *pvm_get_method_by_name(const Klass *klass, const char *name) {
    const uint32_t method_count = klass->mcount;
    for (int i = 0; i < method_count; ++i) {
        Method *method = klass->methods[i];
        if (pvm_symbol_cmp_cstr(method->name, name)) {
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
        GFile *klass_file = g_file_get_child(klass_path_dir, name);
        const size_t t = strlen(name) - FILE_SUFFIX_LEN + 1;
        char klass_name[t];
        snprintf(klass_name, t, "%s", name);
        const bool exist = g_hash_table_contains(vm->klasses, klass_name);
        if (exist) {
            g_clear_object(&klass_file);
            continue;
        }
        Klass *klass = pvm_create_klass(vm, klass_name, klass_file, error);
        g_clear_object(&klass_file);
        if (klass == NULL) {
            goto finally;
        }
        pvm_exec_klass_constructor(klass);
        g_hash_table_insert(vm->klasses, pvm_get_symbol_value(klass->name), klass);
    }
finally:
    g_clear_object(&enumerator);
    g_clear_object(&klass_path_dir);
}
