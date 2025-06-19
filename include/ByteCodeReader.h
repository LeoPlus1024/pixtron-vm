#ifndef BYTECODEREADER_H
#define BYTECODEREADER_H
#include "Type.h"

/**
 * @brief Reads an 8-bit unsigned integer from the VM's code segment
 *
 * Reads 1 byte from the current code segment pointer position and advances
 * the pointer by 1 byte. Typically used for opcode parsing or single-byte immediates.
 *
 * @param context Runtime context containing code segment pointer and VM state
 * @return guint8 The 8-bit unsigned integer value read
 */
extern inline guint8 PixtronVM_ReadByteCodeU8(RuntimeContext *context);

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
extern inline void PixtronVM_ReadByteCodeImm(RuntimeContext *context, VMValue *value);

/**
 * @brief Reads a 32-bit unsigned integer from the VM's code segment
 *
 * Reads 4 bytes from the current code segment pointer position (big-endian format)
 * and advances the pointer by 4 bytes. Used for long immediates or jump offsets.
 *
 * @param context Runtime context containing code segment pointer and VM state
 * @return guint32 The 32-bit unsigned integer value read
 */
extern inline guint32 PixtronVM_ReadByteCodeU32(RuntimeContext *context);

/**
 * @brief Reads a 16-bit unsigned integer from the VM's code segment
 *
 * Reads 2 bytes from the current code segment pointer position (big-endian format)
 * and advances the pointer by 2 bytes. Typically used for short immediates.
 *
 * @param context Runtime context containing code segment pointer and VM state
 * @return guint16 The 16-bit unsigned integer value read
 */
extern inline guint16 PixtronVM_ReadByteCodeU16(RuntimeContext *context);


#endif //BYTECODEREADER_H
