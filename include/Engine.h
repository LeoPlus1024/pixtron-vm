//
// Created by leo on 25-5-27.
//

#ifndef ENGINE_H
#define ENGINE_H
#include "PixotronVM.h"
/**
 * Exec add or sbc operate
 * @param vm {@link PixtronVMPtr}
 * @param sbc sbc?
 */
extern inline void PixotronVM_exec_add_sbc(PixtronVMPtr vm, Opcode opcode);

/**
 * Exec a <b>ifeq</b> 、<b>ifnq</b> 、<b>goto</b> operate
 * @param vm {@link PixtronVMPtr}
 * @param pc Progress count
 * @return Program counter
 */
extern inline uint64_t PixtronVM_exec_jmp(PixtronVMPtr vm, uint64_t pc, Opcode opcode);

/**
 * Exec cmp operate <b>icmp</b>,<b>lcmp</b>,<b>dcmp</b> etc.
 */
extern inline void PixtronVM_exec_cmp(PixtronVMPtr vm, Opcode opcode);

#endif //ENGINE_H
