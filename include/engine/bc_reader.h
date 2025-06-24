#ifndef BC_READER_H
#define BC_READER_H
#include "itype.h"

/**
 * @brief Reads an 8-bit unsigned integer from the VM's code segment
 *
 * Reads 1 byte from the current code segment pointer position and advances
 * the pointer by 1 byte. Typically used for opcode parsing or single-byte immediates.
 *
 * @param context Runtime context containing code segment pointer and VM state
 * @return guint8 The 8-bit unsigned integer value read
 */
extern inline uint8_t pvm_bytecode_read_u8(RuntimeContext *context);

/**
 * @brief Reads a variant-type immediate value from the VM's code segment
 *
 * Reads a value from the code segment according to the variant's type specification,
 * stores the parsed value in the provided variant structure, and advances the code
 * segment pointer by the corresponding byte length (1/2/4 bytes based on type).
 *
 * @param context Runtime context containing code segment pointer and VM state
 * @param variant [out] Destination structure for the parsed variant value
 */
extern inline void pvm_bytecode_read_imm(RuntimeContext *context, Type type, VMValue *value);

/**
 * @brief Reads a 32-bit unsigned integer from the VM's code segment
 *
 * Reads 4 bytes from the current code segment pointer position (big-endian format)
 * and advances the pointer by 4 bytes. Used for long immediates or jump offsets.
 *
 * @param context Runtime context containing code segment pointer and VM state
 * @return guint32 The 32-bit unsigned integer value read
 */
extern inline uint32_t pvm_bytecode_read_u32(RuntimeContext *context);

/**
 * @brief Reads a 16-bit unsigned integer from the VM's code segment
 *
 * Reads 2 bytes from the current code segment pointer position (big-endian format)
 * and advances the pointer by 2 bytes. Typically used for short immediates.
 *
 * @param context Runtime context containing code segment pointer and VM state
 * @return guint16 The 16-bit unsigned integer value read
 */
extern inline uint16_t pvm_bytecode_read_u16(RuntimeContext *context);


#endif //BC_READER_H
