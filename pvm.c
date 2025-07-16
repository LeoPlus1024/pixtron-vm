#include "pvm.h"

#include <assert.h>
#include <locale.h>
#include <stdio.h>

#include "pmem.h"

#include "pklass.h"
#include "pcore.h"
#include "pstr.h"
#include "config.h"
#include "phandle.h"
#include "pstack.h"

extern Value *pvm_create_byte_value(const int8_t i8) {
    VMValue *value = pvm_mem_calloc(VM_VALUE_SIZE);
    value->i8 = i8;
    value->type = TYPE_BYTE;
    return value;
}

extern Value *pvm_create_short_value(const int16_t i16) {
    VMValue *value = pvm_mem_calloc(VM_VALUE_SIZE);
    value->i16 = i16;
    value->type = TYPE_SHORT;
    return value;
}

extern Value *pvm_create_int_value(const int32_t i32) {
    VMValue *value = pvm_mem_calloc(VM_VALUE_SIZE);
    value->i32 = i32;
    value->type = TYPE_INT;
    return value;
}

extern Value *pvm_create_long_value(const int64_t i64) {
    VMValue *value = pvm_mem_calloc(VM_VALUE_SIZE);
    value->i64 = i64;
    value->type = TYPE_LONG;
    return value;
}

extern Value *pvm_create_double_value(const double f64) {
    VMValue *value = pvm_mem_calloc(VM_VALUE_SIZE);
    value->f64 = f64;
    value->type = TYPE_DOUBLE;
    return value;
}

extern int8_t pvm_value_get_byte(const Value *value) {
    assert(value != NULL);
    const VMValue *val = (VMValue *) value;
    assert(val->type == TYPE_BYTE);
    return val->i8;
}

extern int16_t pvm_value_get_short(const Value *value) {
    assert(value != NULL);
    const VMValue *val = (VMValue *) value;
    assert(val->type == TYPE_SHORT);
    return val->i16;
}

extern int32_t pvm_value_get_int(const Value *value) {
    assert(value != NULL);
    const VMValue *val = (VMValue *) value;
    assert(val->type == TYPE_INT);
    return val->i32;
}

extern int64_t pvm_value_get_long(const Value *value) {
    assert(value != NULL);
    const VMValue *val = (VMValue *) value;
    assert(val->type == TYPE_LONG);
    return val->i64;
}

extern double pvm_value_get_double(const Value *value) {
    assert(value != NULL);
    const VMValue *val = (VMValue *) value;
    assert(val->type == TYPE_DOUBLE);
    return val->f64;
}

extern bool pvm_value_get_bool(const Value *value) {
    assert(value != NULL);
    assert(value->type == TYPE_BOOL);
    return value->i32 == 1;
}


extern char *pvm_value_get_string(const Value *value) {
    assert(value != NULL);
    assert(value->type == TYPE_STRING);
    const PStr *pstr = (PStr *) value->obj;
    const uint32_t len = pstr->len;
    if (pstr == NULL || len == 0) {
        NULL;
    }
    void *ptr = pvm_mem_calloc(len + 1);
    memcpy(ptr, pstr->value, len);
    return ptr;
}

extern void pvm_free_string(char **str) {
    if (str == NULL || *str == NULL) {
        return;
    }
    pvm_mem_free(CAST_REF(str));
}

extern Type pvm_get_value_type(const Value *value) {
    return value->type;
}


extern VM *pvm_init(const char *klass_path) {
    // Debug mode enable log output
#if VM_DEBUG_ENABLE
    g_log_set_debug_enabled(TRUE);
#endif
    setlocale(LC_ALL, "");
    PixtronVM *vm = pvm_mem_calloc(sizeof(PixtronVM));
    if (klass_path != NULL) {
        vm->klassPath = g_strdup(klass_path);
    }

    GHashTable *envs = g_hash_table_new(g_str_hash, g_str_equal);
    GHashTable *klasses = g_hash_table_new(g_str_hash, g_str_equal);
    GHashTable *string_table = g_hash_table_new((GHashFunc) pvm_string_hash, (GEqualFunc) pvm_string_equal);
    if (klasses == NULL || envs == NULL) {
        fprintf(stderr, "VM init fail.");
        exit(EXIT_FAILURE);
    }
    char **env_list = g_listenv();
    int32_t index = 0;
    while (true) {
        const char *env = env_list[index++];
        if (env == NULL) {
            break;
        }
        char *value = g_getenv(env);
        if (value == NULL) {
            continue;
        }
        g_hash_table_insert(envs, g_strdup(env), value);
    }
    pvm_mem_free(CAST_REF(env_list));

    vm->envs = envs;
    vm->klasses = klasses;
    vm->string_table = string_table;

    GError *error = NULL;
    pvm_load_system_klass(vm, &error);
    if (error != NULL) {
        pvm_destroy((VM **) &vm);
        g_printerr("%s", error->message);
        g_error_free(error);
        exit(EXIT_FAILURE);
    }
    return (VM *) vm;
}


extern Value *pvm_launch(const VM *vm, const char *klass_name, const char *method_name, uint16_t argc,
                         const Value *argv[]) {
    GError *error = NULL;
    const Klass *klass = pvm_get_klass((PixtronVM *) vm, klass_name, &error);
    if (klass == NULL) {
        if (error != NULL) {
            g_printerr("Launcher VM fail:%s\n", error->message);
        }
        exit(EXIT_FAILURE);
    }
    const Method *method = pvm_get_method_by_name(klass, method_name);
    if (method == NULL) {
        g_printerr("Method '%s' not found in klass:%s", method_name, klass->name);
        exit(EXIT_FAILURE);
    }
    CallMethodParam *method_param = pvm_mem_calloc(sizeof(CallMethodParam));
    method_param->vm = vm;
    method_param->argv = argv;
    method_param->method = method;
    if (argv > 0) {
        method_param->argv = pvm_mem_calloc(sizeof(Value *) * argc);
        for (int i = 0; i < argc; ++i) {
            method_param->argv[i] = (VMValue *) argv[i];
        }
    }
    GThread *thread = g_thread_new("main", (GThreadFunc) pvm_execute, (gpointer) method_param);
    Value *value = g_thread_join(thread);
    if (argv > 0) {
        pvm_mem_free(CAST_REF(&(method_param->argv)));
    }
    pvm_mem_free(TO_REF(method_param));
    return value;
}

extern void pvm_launch_main(const VM *vm, const char *klass_name) {
    pvm_launch(vm, klass_name, "main", 0, NULL);
}

extern Value *pvm_call_vm_method(const char *klass_name, const char *method_name, ...) {
    RuntimeContext *context = pvm_execute_context_get();
    if (context == NULL) {
        g_critical("FATAL: No execution context available when calling method %s::%s",
                   klass_name, method_name);
        exit(EXIT_FAILURE);
    }
    GError *error = NULL;
    const Klass *klass = pvm_get_klass(context->vm, klass_name, &error);
    if (klass == NULL) {
        g_critical("%s\n", error->message);
        exit(EXIT_FAILURE);
    }
    const Method *method = pvm_get_method_by_name(klass, method_name);
    if (method == NULL) {
        g_critical("METHOD NOT FOUND: %s::%s", klass_name, method_name);
        exit(EXIT_FAILURE);
    }
    VirtualStackFrame *call_frame = context->frame;
    const uint16_t argc = method->argc;
    if (argc == 0) {
        return pvm_execute_context(context, call_frame);
    }
    const VirtualStackFrame *frame = pvm_ipush_stack_frame(context, method, argc);
    Value args[argc];
    va_list va_list;
    va_start(va_list, method_name);
    VMValue *locals = GET_LOCALS(frame);
    for (int i = 0; i < argc; ++i) {
        const MethodParam *param = method->argv + i;
        const Type type = param->type;
        VMValue *local = locals + i;
        local->type = type;
        switch (type) {
            case TYPE_INT:
            case TYPE_BOOL:
            case TYPE_BYTE:
            case TYPE_SHORT:
                local->i32 = va_arg(va_list, int);
                break;
            case TYPE_LONG:
                local->i64 = va_arg(va_list, int64_t);
                break;
            case TYPE_DOUBLE:
                local->f64 = va_arg(va_list, double);
                break;
            case TYPE_STRING:
                local->obj = pvm_string_new_from_cstr(va_arg(va_list, void *));
                break;
            case TYPE_HANDLE:
                local->obj = pvm_handle_new(va_arg(va_list, void *), method->m_cleanup);
                break;
            default:
                local->obj = va_arg(va_list, void *);
        }
    }
    va_end(va_list);
    return pvm_execute_context(context, call_frame);
}

extern void pvm_destroy(VM **vm) {
    if (vm == NULL || *vm == NULL) {
        return;
    }
    pvm_mem_free(CAST_REF(vm));
}
