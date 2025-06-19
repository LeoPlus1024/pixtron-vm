#ifndef VIRTUALSTACK_H
#define VIRTUALSTACK_H
#include "Type.h"

/**
 * Push a value onto the VM's operand stack
 * @param vm The PixtronVM instance pointer
 * @param variant The variant value to push onto the stack
 */
extern inline void PixtronVM_PushOperand(RuntimeContext *context, const VMValue *value);

/**
 * Pop a value from the VM's operand stack
 * @param vm The PixtronVM instance pointer
 * @param variant Pointer to store the popped variant value
 */
extern inline VMValue *PixtronVM_PopOperand(RuntimeContext *context);

/**
 * Push a new stack frame onto the call stack
 * @param vm The PixtronVM instance pointer
 * @param maxLocals Maximum number of local variables in the new frame
 * @param maxStack Maximum operand stack size for the new frame
 */
extern inline void PixtronVM_PushStackFrame(RuntimeContext *context, const Method *method);

/**
 * Pop the current stack frame from the call stack
 * @param vm The PixtronVM instance pointer
 */
extern inline void PixtronVM_PopStackFrame(RuntimeContext *context);

/**
 * Set a value in the current frame's local variable table
 * @param vm The PixtronVM instance pointer
 * @param index Local variable table slot index
 * @param variant Variant value to store in the local variable table
 */
extern inline void PixtronVM_SetLocalTable(RuntimeContext *context, uint16_t index, const VMValue *value);

/**
* Get a value from the current frame's local variable table
* @param vm The PixtronVM instance pointer
* @param index Local variable table slot index
* @param variant Pointer to store the retrieved variant value
*/
extern inline void PixtronVM_GetLocalTable(RuntimeContext *context, uint16_t index, VMValue *value);

#endif //VIRTUALSTACK_H
