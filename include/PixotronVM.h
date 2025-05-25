#ifndef PIXOTRONVM_H
#define PIXOTRONVM_H
#include "Type.h"

extern PixtronVMPtr PixtronVM_create(uint8_t *buffer, uint32_t length);


extern void PixtronVM_exec(PixtronVMPtr vm);


extern void PixtronVM_destroy(PixtronVMRef ref);

#endif //PIXOTRONVM_H
