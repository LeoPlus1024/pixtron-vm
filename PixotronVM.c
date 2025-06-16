#include "PixotronVM.h"


#include <stdio.h>
#include <stdlib.h>

#include "Config.h"
#include "Memory.h"
#include "Opcode.h"
#include <time.h>

#include "Klass.h"
#include "DataSegment.h"
#include "VirtualStack.h"
#include "Engine.h"


extern PixtronVM *PixtronVM_create(const gchar *workDir) {
    PixtronVM *vm = PixotronVM_calloc(sizeof(PixtronVM));
    VirtualStackPtr stack = PixotronVM_calloc(sizeof(VirtualStack));
    stack->maxDepth = VM_MAX_STACK_DEPTH;
    stack->depth = 0;
    stack->frame = NULL;
    vm->stack = stack;
    vm->workdir = g_strdup(workDir);

    const GHashTable *classes = g_hash_table_new(g_str_hash, g_str_equal);
    if (classes == NULL) {
        fprintf(stderr, "Classes HashTable init fail.");
        exit(-1);
    }
    vm->klassTable = classes;
    return vm;
}


extern void PixtronVM_exec(PixtronVM *vm, const gchar *clazz, const gchar *func) {
    Klass *klass = PixtronVM_class_get(vm, clazz);
    for (;;) {
        const Opcode ops = PixtronVM_code_segment_u8(vm);

        switch (ops) {
            case PUSH:
                PixtronVM_exec_push(vm);
                break;
            case POP:
                PixtronVM_exec_pop(vm);
                break;
            case ADD:
            case SBC:
                PixotronVM_exec_add_sbc(vm, ops);
                break;
            case GOTO:
            case IFEQ:
            case IFNE:
                PixtronVM_exec_jmp(vm, ops);
                break;
            case ICMP:
            case DCMP:
            case LCMP:
                PixtronVM_exec_cmp(vm, ops);
                break;
            case CONV:
                PixtronVM_exec_conv(vm);
                break;
            case CALL:
                break;
            default:



        }
    }
}

extern void PixtronVM_destroy(PixtronVMRef ref) {
    if (ref == NULL || *ref == NULL) {
        return;
    }
    PixtronVMPtr vm = *ref;
    PixtronVM_stack_frame_pop(vm);
    // PixotronVM_free(TO_REF(vm->header));
    PixotronVM_free(TO_REF(vm->stack));
    //    PixotronVM_free(TO_REF(vm->buffer));
    PixotronVM_free(CAST_REF(ref));
}
