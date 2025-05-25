#ifndef VIRTUALSTACK_H
#define VIRTUALSTACK_H
#include "Type.h"

extern inline void PixtronVM_stack_push(PixtronVMPtr vm, const Variant *variant);

extern inline void PixtronVM_stack_pop(PixtronVMPtr vm, Variant *variant);


extern inline void PixtronVM_stack_frame_push(PixtronVMPtr vm, uint16_t maxLocals, uint16_t maxStack);

extern inline void PixtronVM_stack_frame_pop(PixtronVMPtr vm);


extern inline void PixtronVM_stack_ltable_set(PixtronVMPtr vm, uint16_t index, const VMValue *value);

#endif //VIRTUALSTACK_H
