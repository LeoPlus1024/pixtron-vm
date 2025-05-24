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

#include "VirtualStack.h"


extern PixtronVMPtr PixtronVM_create(const uint8_t *buffer, const uint32_t size) {
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

extern void PixtronVM_exec(PixtronVMPtr pixtron) {
    const uint8_t *buffer = pixtron->buffer;
    const uint32_t size = pixtron->size;
    uint64_t pc = pixtron->pc;
    while (pc < size - 1) {
        const Opcode ops = buffer[pc];

        pc = pc + 1;
        switch (ops) {
            case PUSH: {
                Variant value;
                pc = PixtronVM_ops_data(pixtron, pc, &value);
                PixtronVM_stack_push(pixtron, &value);
                break;
            }
            case POP: {
                Variant value;
                PixtronVM_stack_pop(pixtron, &value);
                uint8_t subOps = buffer[pc];
                const DataSource s = OPS_DATA_SOURCE(subOps);
                pc = pc + 1;
                if (s == GLOBAL_VAR) {
                    const uint32_t *pos = (uint32_t *) (buffer + pc);
                    PixtronVM_gmod(pixtron, pos,OPS_DATA_TYPE(subOps), &value);
                    pc = pc + 5;
                }
            }
            case CALL: {
                break;
            }
            default:



        }
    }
    PixtronVM_stack_frame_pop(pixtron);
}
