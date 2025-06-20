#ifndef VM_H
#define VM_H
#include <stdint.h>

typedef struct VMValue Value;
typedef struct PixtronVM VM;


/**
 * Creates a VMValue containing 8-bit signed integer
 * @param i8 Byte value (-128 to 127)
 * @see PixtronVM_GetByte() for retrieval
 */
extern Value *PixtronVM_CreateByte(int8_t i8);

/**
 * Creates a VMValue containing 16-bit signed integer
 * @param i16 Short value (-32,768 to 32,767)
 * @see PixtronVM_GetShort() for retrieval
 */
extern Value *PixtronVM_CreateShort(int16_t i16);

/**
 * Creates a VMValue containing 32-bit signed integer
 * @param i32 Integer value (-2^31 to 2^31-1)
 * @see PixtronVM_GetInt() for retrieval
 */
extern Value *PixtronVM_CreateInt(int32_t i32);

/**
 * Creates a VMValue containing 64-bit signed integer
 * @param i64 Long value (-2^63 to 2^63-1)
 * @see PixtronVM_GetLong() for retrieval
 */
extern Value *PixtronVM_CreateLong(int64_t i64);

/**
 * Creates a VMValue containing 64-bit floating point
 * @param f64 Double-precision IEEE 754 value
 * @see PixtronVM_GetFloat() for retrieval
 * @note NaN/infinite values preserve their bit patterns
 */
extern Value *PixtronVM_CreateFloat(double f64);


/**
 * Extracts 8-bit integer from VMValue
 * @param value VMValue created with CreateByte()
 */
extern int8_t PixtronVM_GetByte(Value *value);

/**
 * Extracts 16-bit integer from VMValue
 * @param value VMValue created with CreateShort()
 */
extern int16_t PixtronVM_GetShort(Value *value);

/**
 * Extracts 32-bit integer from VMValue
 * @param value VMValue created with CreateInt()
 */
extern int32_t PixtronVM_GetInt(Value *value);

/**
 * Extracts 64-bit integer from VMValue
 * @param value VMValue created with CreateLong()
 */
extern int64_t PixtronVM_GetLong(Value *value);

/**
 * Extracts 64-bit float from VMValue
 * @param value VMValue created with CreateFloat()
 * @note Returns bit-identical NaN values if present
 */
extern double PixtronVM_GetFloat(Value *value);

/**
 * Releases a VMValue and its resources
 * @param value Pointer to VMValue reference (set to NULL after freeing)
 * @note Safe to call with NULL or already freed values
 * @warning Dangling pointer risk if caller retains references
 */
extern void PixtronVM_FreeValue(Value **value);

/**
 * Create a new PixtronVM instance
 *
 * @return Pointer to the newly created PixtronVM instance
 * @note The VM takes ownership of the buffer and will free it when destroyed
 */
extern VM *PixtronVM_CreateVM(const char *klassPath);

/**
 * Execute the loaded bytecode in the virtual machine
 * @param vm Pointer to the PixtronVM instance to execute
 * @note This will run the VM until completion or until an error occurs
 */
extern Value *PixtronVM_LaunchVM(const VM *vm, const char *klassName, uint16_t argv, const Value *args[]);

/**
 * Destroy a PixtronVM instance and release all its resources
 * @param ref Reference to the PixtronVM instance to destroy
 * @note This will also free the associated bytecode buffer
 */
extern void PixtronVM_DestroyVM(VM **vm);


#endif //VM_H
