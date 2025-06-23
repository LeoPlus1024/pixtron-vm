#include "VM.h"
#include <stdio.h>
#include <stdlib.h>

#include "include/engine/Memory.h"

#include "Klass.h"
#include "include/engine/Engine.h"
#include "VMString.h"

extern Value *PixtronVM_CreateByte(const int8_t i8) {
    VMValue *value = PixotronVM_calloc(VM_VALUE_SIZE);
    value->i8 = i8;
    value->type = TYPE_BYTE;
    return (Value *) value;
}

extern Value *PixtronVM_CreateShort(const int16_t i16) {
    VMValue *value = PixotronVM_calloc(VM_VALUE_SIZE);
    value->i16 = i16;
    value->type = TYPE_SHORT;
    return (Value *) value;
}

extern Value *PixtronVM_CreateInt(const int32_t i32) {
    VMValue *value = PixotronVM_calloc(VM_VALUE_SIZE);
    value->i32 = i32;
    value->type = TYPE_INT;
    return (Value *) value;
}

extern Value *PixtronVM_CreateLong(const int64_t i64) {
    VMValue *value = PixotronVM_calloc(VM_VALUE_SIZE);
    value->i64 = i64;
    value->type = TYPE_LONG;
    return (Value *) value;
}

extern Value *PixtronVM_CreateFloat(const double f64) {
    VMValue *value = PixotronVM_calloc(VM_VALUE_SIZE);
    value->f64 = f64;
    value->type = TYPE_DOUBLE;
    return (Value *) value;
}

extern int8_t PixtronVM_GetByte(Value *value) {
    g_assert(value != NULL);
    const VMValue *val = (VMValue *) value;
    g_assert(val->type == TYPE_BYTE);
    return val->i8;
}

extern int16_t PixtronVM_GetShort(Value *value) {
    g_assert(value != NULL);
    const VMValue *val = (VMValue *) value;
    g_assert(val->type == TYPE_SHORT);
    return val->i16;
}

extern int32_t PixtronVM_GetInt(Value *value) {
    g_assert(value != NULL);
    const VMValue *val = (VMValue *) value;
    g_assert(val->type == TYPE_INT);
    return val->i32;
}

extern int64_t PixtronVM_GetLong(Value *value) {
    g_assert(value != NULL);
    const VMValue *val = (VMValue *) value;
    g_assert(val->type == TYPE_LONG);
    return val->i64;
}

extern double PixtronVM_GetDouble(Value *value) {
    g_assert(value != NULL);
    const VMValue *val = (VMValue *) value;
    g_assert(val->type == TYPE_DOUBLE);
    return val->f64;
}


extern VM *PixtronVM_CreateVM(const char *klassPath) {
    PixtronVM *vm = PixotronVM_calloc(sizeof(PixtronVM));
    if (klassPath != NULL) {
        vm->klassPath = g_strdup(klassPath);
    }

    GHashTable *envTable = g_hash_table_new(g_str_hash, g_str_equal);
    GHashTable *klassTable = g_hash_table_new(g_str_hash, g_str_equal);
    GHashTable *strConstantPool =
            g_hash_table_new((GHashFunc) PixtronVM_StringHash, (GEqualFunc) PixtronVM_StringEqual);
    if (klassTable == NULL || envTable == NULL) {
        fprintf(stderr, "VM init fail.");
        exit(-1);
    }
    char **envs = g_listenv();
    int32_t index = 0;
    while (true) {
        const char *env = envs[index++];
        if (env == NULL) {
            break;
        }
        char *value = g_getenv(env);
        if (value == NULL) {
            continue;
        }
        g_hash_table_insert(envTable, g_strdup(env), value);
    }
    vm->envs = envTable;
    vm->klassTable = klassTable;
    vm->strConstantPool = strConstantPool;
    PixotronVM_free(CAST_REF(envs));
    GError *error = NULL;
    PixtronVM_InitSystemKlass(vm, &error);
    if (error != NULL) {
        PixtronVM_DestroyVM((VM **) &vm);
        g_printerr("VM System klass init fail:%s\n", error->message);
        g_error_free(error);
        exit(-1);
    }
    return (VM *) vm;
}


extern Value *PixtronVM_LaunchVM(const VM *vm, const char *klassName, const char *methodName, uint16_t argv,
                                 const Value *args[]) {
    GError *error = NULL;
    const Klass *klass = PixtronVM_GetKlass((PixtronVM *) vm, klassName, &error);
    if (klass == NULL) {
        if (error != NULL) {
            g_printerr("Launcher VM instance fail:%s\n", error->message);
        }
        exit(-1);
    }
    const Method *method = PixtronVM_GetKlassMethodByName(klass, methodName);
    if (method == NULL) {
        g_printerr("Main method not found in klass:%s", klass->name);
        exit(-1);
    }
    CallMethodParam *callMethodParam = PixotronVM_calloc(sizeof(CallMethodParam));
    callMethodParam->argv = argv;
    callMethodParam->method = method;
    if (argv > 0) {
        callMethodParam->args = PixotronVM_calloc(sizeof(Value *) * argv);
        for (int i = 0; i < argv; ++i) {
            callMethodParam->args[i] = (VMValue *) args[i];
        }
    }
    GThread *thread = g_thread_new("Main", (GThreadFunc) PixtronVM_CallMethod, (gpointer) callMethodParam);
    Value *value = g_thread_join(thread);
    if (argv > 0) {
        PixotronVM_free(CAST_REF(&(callMethodParam->args)));
    }
    PixotronVM_free(TO_REF(callMethodParam));
    return value;
}

extern void PixtronVM_DestroyVM(VM **vm) {
    if (vm == NULL || *vm == NULL) {
        return;
    }
    PixotronVM_free(CAST_REF(vm));
}

extern void PixtronVM_FreeValue(Value **value) {
    if (value == NULL) {
        return;
    }
    PixotronVM_free(CAST_REF(value));
}
