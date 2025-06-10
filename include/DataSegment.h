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
extern inline void PixtronVM_data_segment_set(PixtronVMPtr vm, const uint32_t offset, const Type type,
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
extern inline void PixtronVM_data_segment_get(PixtronVMPtr vm, uint32_t offset, Variant *variant);


/**
 * @brief Reads an 8-bit unsigned integer from VM's code segment
 *
 * Reads 1 byte from the current code segment pointer position
 * and advances the pointer by 1 byte.
 *
 * @param vm Pointer to the PixtronVM instance
 * @return uint8_t The 8-bit unsigned integer value read
 */
extern inline uint8_t PixtronVM_code_segment_u8(PixtronVMPtr vm);

/**
 * @brief Reads a variant-type immediate value from VM's code segment
 *
 * Reads a variant value from the current code segment pointer position
 * and advances the pointer according to the variant's type size.
 *
 * @param vm Pointer to the PixtronVM instance
 * @param variant Pointer to store the read variant value
 */
extern inline void PixtronVM_code_segment_imm(PixtronVMPtr vm, Variant *variant);

/**
 * @brief Reads a 32-bit unsigned integer from VM's code segment
 *
 * Reads 4 bytes from the current code segment pointer position
 * and advances the pointer by 4 bytes.
 *
 * @param vm Pointer to the PixtronVM instance
 * @return uint32_t The 32-bit unsigned integer value read
 */
extern inline uint32_t PixtronVM_code_segment_u32(PixtronVMPtr vm);

/**
 * @brief Reads a 16-bit unsigned integer from VM's code segment
 *
 * Reads 2 bytes from the current code segment pointer position
 * and advances the pointer by 2 bytes.
 *
 * @param vm Pointer to the PixtronVM instance
 * @return uint16_t The 16-bit unsigned integer value read
 */
extern inline uint16_t PixtronVM_code_segment_u16(PixtronVMPtr vm);

#endif //STATICDATA_H
