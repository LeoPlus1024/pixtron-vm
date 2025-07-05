#ifndef BC_READER_H
#define BC_READER_H
#include "ptype.h"

/**
 * @brief Reads an 8-bit signed integer from the VM's code segment
 *
 * Reads 1 byte from the current code segment pointer position and advances
 * the pointer by 1 byte. Typically used for opcode parsing or single-byte immediates.
 *
 * @param context Runtime context containing code segment pointer and VM state
 * @return guint8 The 8-bit unsigned integer value read
 */
extern inline int8_t pvm_bytecode_read_int8(const RuntimeContext *context);

/**
 * @brief Reads a 32-bit signed integer from the VM's code segment
 *
 * Reads 4 bytes from the current code segment pointer position (big-endian format)
 * and advances the pointer by 4 bytes. Used for long immediates or jump offsets.
 *
 * @param context Runtime context containing code segment pointer and VM state
 * @return guint32 The 32-bit unsigned integer value read
 */
extern inline int32_t pvm_bytecode_read_int32(const RuntimeContext *context);

/**
 * @brief Reads a 16-bit signed integer from the VM's code segment
 *
 * Reads 2 bytes from the current code segment pointer position (big-endian format)
 * and advances the pointer by 2 bytes. Typically used for short immediates.
 *
 * @param context Runtime context containing code segment pointer and VM state
 * @return guint16 The 16-bit unsigned integer value read
 */
extern inline int16_t pvm_bytecode_read_int16(const RuntimeContext *context);


extern inline int64_t pvm_bytecode_read_int64(const RuntimeContext *context);

extern inline double pvm_bytecode_read_f64(const RuntimeContext *context);

#endif //BC_READER_H
