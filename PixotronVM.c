#include "PixotronVM.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Config.h"
#include "Memory.h"
#include "Opncode.h"
#include "Program.h"
#include <stdint.h>

#include "Engine.h"
#include "VirtualStack.h"


extern PixtronVMPtr PixtronVM_create(uint8_t *buffer, uint32_t size) {
    PixtronVMPtr pixtron = PixotronVM_calloc(sizeof(PixtronVM));
    VirtualStackPtr stack = PixotronVM_calloc(sizeof(VirtualStack));
    stack->maxDepth = VM_MAX_STACK_DEPTH;
    stack->depth = 0;
    stack->frame = NULL;
    pixtron->size = size;
    pixtron->stack = stack;
    pixtron->buffer = buffer;

    bool success = PixtronVM_init(pixtron);

    if (!success) {
        fprintf(stderr, "Failed to initialize PixtronVM\n");
        exit(-1);
    }

    return pixtron;
}


static uint32_t PixtronVM_ops_data(PixtronVMPtr pixtron, const uint64_t pc, Variant *variant) {
    const uint8_t *buffer = pixtron->buffer;
    const uint8_t subOps = buffer[pc];
    const DataType type = OPS_DATA_TYPE(subOps);
    const DataSource source = OPS_DATA_SOURCE(subOps);
    const uint8_t size = TYPE_SIZE[type];
    if (source == IMMEDIATE) {
        if (size == 0 || (size + pc + 1) > pixtron->size) {
            fprintf(stderr, "Opcode data width error.");
            exit(-1);
        }

        memcpy(&variant->value, &buffer[pc + 1], size);
    }
    // 全局变量
    else if (source == GLOBAL_VAR) {
        uint8_t b = 10;
    }
    variant->type = type;
    return pc + size + 1;
}

static void PixtronVM_gmod(PixtronVMPtr pixtron, const uint32_t *pos, const DataType type, const Variant *variant) {
    assert(type == variant->type);
    const uint32_t index = *pos;
    uint8_t *ptr = pixtron->data + index;
    const uint8_t size = TYPE_SIZE[type];
    memcpy(ptr, &variant->value, size);
}

extern void PixtronVM_exec(PixtronVMPtr vm) {
    const uint8_t *buffer = vm->buffer;
    const uint32_t size = vm->size;
    uint64_t pc = vm->pc;
    while (pc < size) {
        const Opcode ops = buffer[pc];

        pc = pc + 1;
        switch (ops) {
            case PUSH: {
                Variant value;
                pc = PixtronVM_ops_data(vm, pc, &value);
                PixtronVM_stack_push(vm, &value);
                break;
            }
            case POP: {
                Variant value;
                PixtronVM_stack_pop(vm, &value);
                uint8_t subOps = buffer[pc];
                const DataSource s = OPS_DATA_SOURCE(subOps);
                pc = pc + 1;
                if (s == GLOBAL_VAR) {
                    const uint32_t *pos = (uint32_t *) (buffer + pc);
                    PixtronVM_gmod(vm, pos,OPS_DATA_TYPE(subOps), &value);
                    pc = pc + 5;
                }
            }
            case ADD: {
                PixotronVM_exec_add(vm);
                break;
            }
            case CALL: {
                break;
            }
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
    PixotronVM_free(TO_REF(vm->header));
    PixotronVM_free(TO_REF(vm->stack));
    //    PixotronVM_free(TO_REF(vm->buffer));
    PixotronVM_free(CAST_REF(ref));
}
