#include "pvm.h"

#include <locale.h>
#include <stdio.h>

#include "pmem.h"

#include "pklass.h"
#include "pcore.h"
#include "pstr.h"
#include "config.h"

extern Value *pvm_create_byte_value(const int8_t i8) {
    VMValue *value = pvm_mem_calloc(VM_VALUE_SIZE);
    value->i8 = i8;
    value->type = TYPE_BYTE;
    return (Value *) value;
}

extern Value *pvm_create_short_value(const int16_t i16) {
    VMValue *value = pvm_mem_calloc(VM_VALUE_SIZE);
    value->i16 = i16;
    value->type = TYPE_SHORT;
    return (Value *) value;
}

extern Value *pvm_create_int_value(const int32_t i32) {
    VMValue *value = pvm_mem_calloc(VM_VALUE_SIZE);
    value->i32 = i32;
    value->type = TYPE_INT;
    return (Value *) value;
}

extern Value *pvm_create_long_value(const int64_t i64) {
    VMValue *value = pvm_mem_calloc(VM_VALUE_SIZE);
    value->i64 = i64;
    value->type = TYPE_LONG;
    return (Value *) value;
}

extern Value *pvm_create_double_value(const double f64) {
    VMValue *value = pvm_mem_calloc(VM_VALUE_SIZE);
    value->f64 = f64;
    value->type = TYPE_DOUBLE;
    return (Value *) value;
}

extern int8_t pvm_value_get_byte(const Value *value) {
    g_assert(value != NULL);
    const VMValue *val = (VMValue *) value;
    g_assert(val->type == TYPE_BYTE);
    return val->i8;
}

extern int16_t pvm_value_get_short(const Value *value) {
    g_assert(value != NULL);
    const VMValue *val = (VMValue *) value;
    g_assert(val->type == TYPE_SHORT);
    return val->i16;
}

extern int32_t pvm_value_get_int(const Value *value) {
    g_assert(value != NULL);
    const VMValue *val = (VMValue *) value;
    g_assert(val->type == TYPE_INT);
    return val->i32;
}

extern int64_t pvm_value_get_long(const Value *value) {
    g_assert(value != NULL);
    const VMValue *val = (VMValue *) value;
    g_assert(val->type == TYPE_LONG);
    return val->i64;
}

extern double pvm_value_get_double(const Value *value) {
    g_assert(value != NULL);
    const VMValue *val = (VMValue *) value;
    g_assert(val->type == TYPE_DOUBLE);
    return val->f64;
}


extern const char *pvm_value_get_string(const Value *value) {
    g_assert(value != NULL);
    g_assert(value->type == TYPE_STRING);
    const PStr *str = (PStr *) value->obj;
    if (str == NULL) {
        NULL;
    }
    return str->str;
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
    GHashTable *string_constants =
            g_hash_table_new((GHashFunc) pvm_string_hash, (GEqualFunc) pvm_string_equal);
    if (klasses == NULL || envs == NULL) {
        fprintf(stderr, "VM init fail.");
        exit(-1);
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
    vm->string_constants = string_constants;

    GError *error = NULL;
    pvm_load_system_klass(vm, &error);
    if (error != NULL) {
        pvm_destroy((VM **) &vm);
        g_printerr("Virtual machine init fail: %s\n", error->message);
        g_error_free(error);
        exit(-1);
    }
    return (VM *) vm;
}


extern Value *pvm_launch(const VM *vm, const char *klass_name, const char *method_name, uint16_t argv,
                         const Value *args[]) {
    GError *error = NULL;
    const Klass *klass = pvm_get_klass((PixtronVM *) vm, klass_name, &error);
    if (klass == NULL) {
        if (error != NULL) {
            g_printerr("Launcher VM fail:%s\n", error->message);
        }
        exit(-1);
    }
    const Method *method = pvm_get_method_by_name(klass, method_name);
    if (method == NULL) {
        g_printerr("Method '%s' not found in klass:%s", method_name, klass->name);
        exit(-1);
    }
    CallMethodParam *method_param = pvm_mem_calloc(sizeof(CallMethodParam));
    method_param->argv = argv;
    method_param->method = method;
    if (argv > 0) {
        method_param->args = pvm_mem_calloc(sizeof(Value *) * argv);
        for (int i = 0; i < argv; ++i) {
            method_param->args[i] = (VMValue *) args[i];
        }
    }
    GThread *thread = g_thread_new("Main Thread", (GThreadFunc) pvm_call_method, (gpointer) method_param);
    Value *value = g_thread_join(thread);
    if (argv > 0) {
        pvm_mem_free(CAST_REF(&(method_param->args)));
    }
    pvm_mem_free(TO_REF(method_param));
    return value;
}

extern void pvm_launch_main(const VM *vm, const char *klass_name) {
    pvm_launch(vm, klass_name, "main", 0, NULL);
}

extern void pvm_destroy(VM **vm) {
    if (vm == NULL || *vm == NULL) {
        return;
    }
    pvm_mem_free(CAST_REF(vm));
}

extern void pvm_free_value(Value **value) {
    if (value == NULL) {
        return;
    }
    pvm_mem_free(CAST_REF(value));
}

extern void pvm_free_values(const Value *value[], const uint64_t length) {
    if (value == NULL) {
        return;
    }
    for (uint64_t i = 0; i < length; i++) {
        pvm_mem_free(TO_REF(value[i]));
    }
}
