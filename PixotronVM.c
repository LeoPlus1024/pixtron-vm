#include "PixotronVM.h"


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "Memory.h"

#include "Klass.h"
#include "Engine.h"


extern PixtronVM *PixtronVM_CreateVM(const gchar *workDir) {
    PixtronVM *vm = PixotronVM_calloc(sizeof(PixtronVM));

    vm->workdir = g_strdup(workDir);

    GHashTable *klassTable = g_hash_table_new(g_str_hash, g_str_equal);
    if (klassTable == NULL) {
        fprintf(stderr, "Classes HashTable init fail.");
        exit(-1);
    }
    vm->klassTable = klassTable;
    return vm;
}


extern void PixtronVM_LaunchVM(const PixtronVM *vm, const gchar *clazzName) {
    GError *error = NULL;
    const Klass *klass = PixtronVM_GetKlass(vm, clazzName, &error);
    if (klass == NULL) {
        if (error != NULL) {
            g_error_free(error);
        }
        g_thread_exit(NULL);
    }
    const Method *method = PixtronVM_GetKlassMethod(klass, "main");
    if (method == NULL) {
        g_printerr("Main method not found in klass:%s", klass->name);
        g_thread_exit(NULL);
    }
    GThread *thread = g_thread_new("main", (GThreadFunc) PixtronVM_CallMethod, (gpointer) method);

    g_thread_join(thread);
}

extern void PixtronVM_DestroyVM(PixtronVM **vm) {
    if (vm == NULL || *vm == NULL) {
        return;
    }
    PixtronVMPtr ptr = *vm;
    // PixotronVM_free(TO_REF(vm->header));
    PixotronVM_free(TO_REF(ptr->stack));
    //    PixotronVM_free(TO_REF(vm->buffer));
    PixotronVM_free(CAST_REF(vm));
}
