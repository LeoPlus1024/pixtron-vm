#ifndef PIXOTRONVM_H
#define PIXOTRONVM_H
#include "Type.h"

extern PixtronVMPtr PixtronVM_create(const uint8_t *buffer, uint32_t length);


extern void PixtronVM_exec(PixtronVMPtr pixtron);

#endif //PIXOTRONVM_H
