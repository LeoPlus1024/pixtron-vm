#ifndef STATICDATA_H
#define STATICDATA_H
#include "Type.h"

/**
 * Set variant value to data segment
 * @param vm PixotronVM pointer
 * @param offset Data segment offset
 * @param type Data type
 * @param variant {@link Variant}
 */
extern void inline PixtronVM_data_segment_set(PixtronVMPtr vm, const uint32_t *offset, DataType type,
                                              const Variant *variant);

/**
 * Get variable from data segment
 * @param vm {@link PixtronVMPtr}
 * @param offset Data segment offset
 * @param variant {@link Variant}
 */
extern void inline PixtronVM_data_segment_get(PixtronVMPtr vm, const uint32_t *offset, Variant *variant);

#endif //STATICDATA_H
