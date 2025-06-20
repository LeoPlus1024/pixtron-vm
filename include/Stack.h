#ifndef VIRTUALSTACK_H
#define VIRTUALSTACK_H
#include "Type.h"

/**
 * Pushes a value onto the RuntimeContext's operand stack.
 *
 * @param context Pointer to the RuntimeContext containing the operand stack
 * @param value Pointer to the VMValue to be pushed. If NULL, only the stack pointer (sp)
 *              will be decremented (since the stack grows downward) without modifying the
 *              stack contents. If non-NULL, the value will be copied to the stack at the
 *              new sp position (after decrementing), then sp will be updated.
 *
 * @note This function assumes the stack has sufficient capacity. It is the caller's
 *       responsibility to ensure stack bounds are not exceeded.
 * @note The stack pointer (sp) is decremented **before** storing the value (if non-NULL),
 *       following the typical downward-growing stack convention.
 * @warning Passing a context with a NULL operand stack will result in undefined behavior.
 */
extern inline void PixtronVM_PushOperand(RuntimeContext *context, const VMValue *value);


/**
 * Pops a value from the RuntimeContext's operand stack.
 *
 * @param context Pointer to the RuntimeContext containing the operand stack.
 * @return Pointer to the popped VMValue. If the stack is empty (sp at bottom),
 *         returns NULL and sets the VM's runtime error flag to STACK_UNDERFLOW.
 *         If the stack pointer (sp) is invalid (corrupted or overflowed),
 *         triggers a fatal STACK_OVERFLOW error and terminates the **current thread**,
 *         while keeping the rest of the VM running.
 *
 * @note The stack pointer (sp) is incremented (moved upward) **after** retrieving the value,
 *       following the downward-growing stack convention (sp decreases on push, increases on pop).
 * @note This function performs minimal stack bounds checking. Popping from an empty stack
 *       is safe (returns NULL + error flag), but attempting to pop beyond the stack's
 *       allocated bounds will terminate the current thread.
 * @warning Passing a NULL context results in undefined behavior.
 * @see PixtronVM_CheckStackBounds
 */
extern inline VMValue *PixtronVM_PopOperand(RuntimeContext *context);


/**
 * Pushes a new stack frame onto the call stack for method invocation.
 *
 * @param context      Runtime context containing the call stack (non-NULL)
 * @param method       Method metadata for the invoked function (non-NULL)
 * <pre>
 * @frame_layout
 * +---------------------------+
 * | Saved FP (caller's frame) |  // Maintains linked stack trace
 * | Previous Frame* (pre)     |  // Enables O(1) frame popping
 * | Caller PC (uint32_t)      |  // Required for stack unwinding
 * +---------------------------+
 * | Method Parameters         |  // Copied from caller's operand stack
 * | Local Variables           |  // Zero-initialized slots
 * | Operand Stack             |  // Grows downward from frame base
 * +---------------------------+
 * </pre>
 * @key_features
 * 1. Tail Call Optimization (TCO) Support:
 *    - Current frame can be reused when:
 *      a) The call is in tail position
 *      b) Callee's parameter count <= current frame size
 *    - Optimization workflow:
 *      1) Overwrite parameters in-place
 *      2) Update pre pointer to target method's frame
 *      3) Jump to callee (no new frame allocation)
 *
 * 2. Debugging Capabilities:
 *    - Complete stack traces via pre pointers
 *    - Precise PC tracking for crash reports
 *
 * @performance_characteristics
 * - Frame allocation: 1 indirect branch + 2 pointer writes
 * - Tail call overhead: 3 instructions (pointer swap + jump)
 * - Memory overhead: 8 bytes per frame (pre+PC on 32-bit)
 *
 * @safety_contracts
 * - Requires: VALID_CONTEXT(context) && VALID_METHOD(method)
 * - Ensures:
 *   1) context->fp points to new frame
 *   2) All local slots initialized to VM_NULL
 *   3) Operand stack empty in new frame
 *
 */
extern inline void PixtronVM_PushStackFrame(RuntimeContext *context, const Method *method, uint16_t argv,const VMValue *args);


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
