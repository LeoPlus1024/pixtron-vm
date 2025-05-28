#include "PixotronVM.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Config.h"
#include "Memory.h"
#include "Opcode.h"
#include <stdint.h>

#include "DataSegment.h"
#include "VirtualStack.h"
#include "IError.h"
#include "Engine.h"


#define MAGIC  (0xFFAABBCC)


static bool PixtronVM_init(PixtronVMPtr vm) {
    uint8_t *buffer = vm->buffer;
    uint32_t magic = *((uint32_t *) buffer);
    if (magic != MAGIC) {
        fprintf(stderr, "Magic not match except %d but it is %d\n",MAGIC, magic);
        errno = MAGIC_ERROR;
        return false;
    }
    const Version version = *((Version *) (buffer + 4));
    const uint32_t dataOffset = *((uint32_t *) (buffer + 6)) + 18;
    const uint32_t dataLength = *((uint32_t *) (buffer + 10)) + 18;
    const uint32_t codeOffset = *((uint32_t *) (buffer + 14)) + 18;

    BinaryHeaderPtr header = PixotronVM_calloc(sizeof(BinaryHeader));

    header->magic = MAGIC;
    header->version = version;
    header->dataOffset = dataOffset;
    header->dataLength = dataLength;
    header->codeOffset = codeOffset;
    vm->pc = codeOffset;

    vm->header = header;
    vm->data = buffer + dataOffset;

    PixtronVM_stack_frame_push(vm, 10, 10);

    return true;
}


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


static uint32_t PixtronVM_ops_data(PixtronVMPtr vm, const uint64_t pc, Variant *variant) {
    const uint8_t *buffer = vm->buffer;
    const uint8_t subOps = buffer[pc];
    const Type type = OPS_DATA_TYPE(subOps);
    const DataSource source = OPS_DATA_SOURCE(subOps);
    variant->type = type;
    uint64_t tmp = pc + 1;
    // 立即数
    if (source == IMMEDIATE) {
        const uint8_t size = TYPE_SIZE[type];
        if (size == 0 || (size + tmp) > vm->size) {
            fprintf(stderr, "Opcode data width error.");
            exit(-1);
        }
        memcpy(&variant->value, &buffer[tmp], size);
        tmp = tmp + size;
    }
    // 全局变量
    else if (source == GLOBAL_VAR) {
        const uint32_t *pos = (uint32_t *) buffer + tmp;
        PixtronVM_data_segment_get(vm, pos, variant);
        tmp = tmp + 4;
    }
    return tmp;
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
                const uint8_t subOps = buffer[pc];
                const DataSource s = OPS_DATA_SOURCE(subOps);
                pc = pc + 1;
                // 设置全局变量
                if (s == GLOBAL_VAR) {
                    const uint32_t *pos = (uint32_t *) (buffer + pc);
                    PixtronVM_data_segment_set(vm, pos,OPS_DATA_TYPE(subOps), &value);
                    pc = pc + 5;
                }
                // 设置局部变量
                else if (s == LOCAL_VAR) {
                }
            }
            case ADD:
            case SBC: {
                PixotronVM_exec_add_sbc(vm, ops);
                break;
            }
            case GOTO:
            case IFEQ:
            case IFNE: {
                PixtronVM_exec_jmp(vm, pc, ops);
                break;
            }
            case ICMP:
            case DCMP:
            case LCMP:
                PixtronVM_exec_cmp(vm, ops);
                break;
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
