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
#include <time.h>

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

    const bool success = PixtronVM_init(pixtron);

    if (!success) {
        fprintf(stderr, "Failed to initialize PixtronVM\n");
        exit(-1);
    }

    return pixtron;
}


extern void PixtronVM_exec(PixtronVMPtr vm) {
    const uint32_t size = vm->size;

    while (vm->pc < size) {
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
    PixotronVM_free(TO_REF(vm->header));
    PixotronVM_free(TO_REF(vm->stack));
    //    PixotronVM_free(TO_REF(vm->buffer));
    PixotronVM_free(CAST_REF(ref));
}
