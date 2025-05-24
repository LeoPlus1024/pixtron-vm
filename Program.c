//
// Created by leo on 25-5-23.
//

#include "Program.h"

#include <errno.h>
#include <stdio.h>

#include "IError.h"
#include "Memory.h"

#define MAGIC  (0xFFAABBCC)


extern bool PixtronVM_init(PixtronVMPtr pixtron) {
    uint8_t *buffer = pixtron->buffer;
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
    pixtron->pc = codeOffset;

    pixtron->header = header;
    pixtron->data = buffer + dataOffset;

    return true;
}
