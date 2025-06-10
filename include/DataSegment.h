#ifndef STATICDATA_H
#define STATICDATA_H
#include "Type.h"

/**
 * Set a variant value to the VM's data segment at specified offset
 * @param vm Pointer to the PixtronVM instance
 * @param offset Pointer to the offset location in data segment (must be within bounds)
 * @param type Expected data type for type checking (use TYPE_UNKNOWN to bypass checking)
 * @param variant Pointer to the source Variant containing the value to set
 * @note Performs implicit type conversion if the variant type doesn't match the segment type
 * @warning Writing beyond data segment bounds will trigger a VM fault
 * @see PixtronVM_data_segment_get
 */
extern inline void PixtronVM_data_segment_set(PixtronVMPtr vm, const uint32_t *offset, const Type type,
                                              const Variant *variant);

/**
 * Retrieve a variable from the VM's data segment at specified offset
 * @param vm Pointer to the PixtronVM instance
 * @param offset Pointer to the offset location in data segment (must be within bounds)
 * @param variant Pointer to the destination Variant to store the retrieved value
 * @note The variant's type will be updated to reflect the retrieved data type
 * @warning Reading beyond data segment bounds will trigger a VM fault
 * @see PixtronVM_data_segment_set
 */
extern inline void PixtronVM_data_segment_get(PixtronVMPtr vm, const uint32_t *offset, Variant *variant);
#endif //STATICDATA_H
