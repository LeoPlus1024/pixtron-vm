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


extern PixtronVM *PixtronVM_CreateVM(const gchar *workDir) {
    PixtronVM *vm = PixotronVM_calloc(sizeof(PixtronVM));
    VirtualStackPtr stack = PixotronVM_calloc(sizeof(VirtualStack));
    stack->maxDepth = VM_MAX_STACK_DEPTH;
    stack->depth = 0;
    stack->frame = NULL;
    vm->stack = stack;
    vm->workdir = g_strdup(workDir);

    GHashTable *klassTable = g_hash_table_new(g_str_hash, g_str_equal);
    if (klassTable == NULL) {
        fprintf(stderr, "Classes HashTable init fail.");
        exit(-1);
    }
    vm->klassTable = klassTable;
    return vm;
}


extern void PixtronVM_Exec(PixtronVM *vm, const gchar *clazz, const gchar *func) {
    Klass *klass = PixtronVM_GetKlass(vm, clazz);
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

extern void PixtronVM_DestroyVM(PixtronVM **vm) {
    if (vm == NULL || *vm == NULL) {
        return;
    }
    PixtronVMPtr ptr = *vm;
    PixtronVM_stack_frame_pop(ptr);
    // PixotronVM_free(TO_REF(vm->header));
    PixotronVM_free(TO_REF(ptr->stack));
    //    PixotronVM_free(TO_REF(vm->buffer));
    PixotronVM_free(CAST_REF(vm));
}
