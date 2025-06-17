#ifndef BYTECODEREADER_H
#define BYTECODEREADER_H
#include "Type.h"

/**
 * @brief Reads an 8-bit unsigned integer from VM's code segment
 *
 * Reads 1 byte from the current code segment pointer position
 * and advances the pointer by 1 byte.
 *
 * @param vm Pointer to the PixtronVM instance
 * @return uint8_t The 8-bit unsigned integer value read
 */
extern inline uint8_t PixtronVM_ReadByteCodeU8(const RuntimeContext *context);

/**
 * @brief Reads a variant-type immediate value from VM's code segment
 *
 * Reads a variant value from the current code segment pointer position
 * and advances the pointer according to the variant's type size.
 *
 * @param vm Pointer to the PixtronVM instance
 * @param variant Pointer to store the read variant value
 */
extern inline void PixtronVM_ReadByteCodeImm(RuntimeContext *context, Variant *variant);

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
extern inline uint16_t PixtronVM_ReadByteCodeU16(RuntimeContext *context);

#endif //BYTECODEREADER_H
