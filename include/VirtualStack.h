#ifndef VIRTUALSTACK_H
#define VIRTUALSTACK_H
#include "Type.h"

/**
 * Push a value onto the VM's operand stack
 * @param vm The PixtronVM instance pointer
 * @param variant The variant value to push onto the stack
 */
extern inline void PixtronVM_stack_push(PixtronVMPtr vm, const Variant *variant);

/**
 * Pop a value from the VM's operand stack
 * @param vm The PixtronVM instance pointer
 * @param variant Pointer to store the popped variant value
 */
extern inline void PixtronVM_stack_pop(PixtronVMPtr vm, Variant *variant);

/**
 * Push a new stack frame onto the call stack
 * @param vm The PixtronVM instance pointer
 * @param maxLocals Maximum number of local variables in the new frame
 * @param maxStack Maximum operand stack size for the new frame
 */
extern inline void PixtronVM_stack_frame_push(PixtronVMPtr vm, uint16_t maxLocals, uint16_t maxStack);

/**
 * Pop the current stack frame from the call stack
 * @param vm The PixtronVM instance pointer
 */
extern inline void PixtronVM_stack_frame_pop(PixtronVMPtr vm);

/**
 * Set a value in the current frame's local variable table
 * @param vm The PixtronVM instance pointer
 * @param index Local variable table slot index
 * @param variant Variant value to store in the local variable table
 */
extern inline void PixtronVM_stack_ltable_set(PixtronVMPtr vm, uint16_t index, const Variant *variant);

#endif //VIRTUALSTACK_H
