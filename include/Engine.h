#ifndef ENGINE_H
#define ENGINE_H
#include "PixotronVM.h"
// /**
//  * Exec add or sbc operate
//  * @param vm {@link PixtronVMPtr}
//  * @param sbc sbc?
//  */
// extern inline void PixotronVM_exec_add_sbc(PixtronVMPtr vm, Opcode opcode);
//
// /**
//  * Exec a <b>ifeq</b> 、<b>ifnq</b> 、<b>goto</b> operate
//  * @param vm {@link PixtronVMPtr}
//  * @param pc Progress count
//  * @return Program counter
//  */
// extern inline void PixtronVM_exec_jmp(PixtronVMPtr vm, Opcode opcode);
//
// /**
//  * Exec cmp operate <b>icmp</b>,<b>lcmp</b>,<b>dcmp</b> etc.
//  */
// extern inline void PixtronVM_exec_cmp(PixtronVMPtr vm, Opcode opcode);
//
// /**
//  * Exec <b>conv</b> operate for stack top value
//  */
// extern inline void PixtronVM_exec_conv(PixtronVMPtr vm);
//
// /**
//  * Exec stack pop operation
//  * @param vm {@link PixtronVMPtr} - Pointer to the virtual machine instance
//  * Removes the top value from the virtual machine's operand stack
//  */
// extern inline void PixtronVM_exec_pop(PixtronVMPtr vm);
//
// /**
//  * Exec stack push operation
//  * @param vm {@link PixtronVMPtr} - Pointer to the virtual machine instance
//  * Pushes a value onto the virtual machine's operand stack
//  */
// extern inline void PixtronVM_exec_push(PixtronVMPtr vm);
//

extern Variant *PixtronVM_CallMethod(const Method *method);

#endif //ENGINE_H
