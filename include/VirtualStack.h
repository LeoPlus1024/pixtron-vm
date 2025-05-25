//
// Created by leo on 25-5-24.
//

#ifndef VIRTUALSTACK_H
#define VIRTUALSTACK_H
#include "Type.h"

extern inline void PixtronVM_stack_push(PixtronVMPtr vm, const Variant *variant);

extern inline void PixtronVM_stack_pop(PixtronVMPtr vm, Variant *variant);


extern inline void PixtronVM_stack_frame_push(PixtronVMPtr vm, size_t maxLocals, size_t maxStack);

extern inline void PixtronVM_stack_frame_pop(PixtronVMPtr vm);

#endif //VIRTUALSTACK_H
