//
// Created by leo on 25-5-25.
//

#ifndef ENGINE_H
#define ENGINE_H
#include <stdbool.h>

#include "PixotronVM.h"

/**
 * Exec add or sbc operate
 * @param vm {@link PixtronVMPtr}
 * @param sbc sbc?
 */
extern inline void PixotronVM_exec_add_sbc(PixtronVMPtr vm,bool sbc);

#endif //ENGINE_H
