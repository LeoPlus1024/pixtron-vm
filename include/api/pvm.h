#ifndef VM_H
#define VM_H
#include <stdbool.h>
#include <stdint.h>

// 类型标记（操作数中使用）
typedef enum:uint16_t {
    NIL = 0,
    TYPE_BYTE,
    TYPE_SHORT,
    TYPE_INT,
    TYPE_LONG,
    TYPE_DOUBLE,
    TYPE_BOOL,
    TYPE_REF,
    TYPE_VOID,
    TYPE_STRING,
    TYPE_ARRAY,
    TYPE_OBJECT,
} Type;


static const uint8_t TYPE_SIZE[] = {
    [NIL] = 0,
    [TYPE_BYTE] = 1,
    [TYPE_SHORT] = 2,
    [TYPE_INT] = 4,
    [TYPE_LONG] = 8,
    [TYPE_DOUBLE] = 8,
    [TYPE_BOOL] = 1,
    [TYPE_REF] = 8,
    [TYPE_ARRAY] = 8,
};

static const char *const TYPE_NAME[] = {
    "null",
    "byte",
    "short",
    "int",
    "long",
    "double",
    "bool",
    "object",
    "void",
    "string",
    "array"
};

typedef struct _VMValue Value;
typedef struct PixtronVM VM;


/**
 * Creates a VMValue containing 8-bit signed integer
 * @param i8 Byte value (-128 to 127)
 * @see PixtronVM_GetByte() for retrieval
 */
extern Value *pvm_create_byte_value(int8_t i8);

/**
 * Creates a VMValue containing 16-bit signed integer
 * @param i16 Short value (-32,768 to 32,767)
 * @see PixtronVM_GetShort() for retrieval
 */
extern Value *pvm_create_short_value(int16_t i16);

/**
 * Creates a VMValue containing 32-bit signed integer
 * @param i32 Integer value (-2^31 to 2^31-1)
 * @see PixtronVM_GetInt() for retrieval
 */
extern Value *pvm_create_int_value(int32_t i32);

/**
 * Creates a VMValue containing 64-bit signed integer
 * @param i64 Long value (-2^63 to 2^63-1)
 * @see PixtronVM_GetLong() for retrieval
 */
extern Value *pvm_create_long_value(int64_t i64);

/**
 * Creates a VMValue containing 64-bit floating point
 * @param f64 Double-precision IEEE 754 value
 * @see PixtronVM_GetFloat() for retrieval
 * @note NaN/infinite values preserve their bit patterns
 */
extern Value *pvm_create_double_value(double f64);


/**
 * Extracts 8-bit integer from VMValue
 * @param value VMValue created with CreateByte()
 */
extern int8_t pvm_value_get_byte(const Value *value);

/**
 * Extracts 16-bit integer from VMValue
 * @param value VMValue created with CreateShort()
 */
extern int16_t pvm_value_get_short(const Value *value);

/**
 * Extracts 32-bit integer from VMValue
 * @param value VMValue created with CreateInt()
 */
extern int32_t pvm_value_get_int(const Value *value);

/**
 * Extracts 64-bit integer from VMValue
 * @param value VMValue created with CreateLong()
 */
extern int64_t pvm_value_get_long(const Value *value);

/**
 * Extracts 64-bit float from VMValue
 * @param value VMValue created with CreateFloat()
 * @note Returns bit-identical NaN values if present
 */
extern double pvm_value_get_double(const Value *value);


extern bool pvm_value_get_bool(const Value *value);

extern const char *pvm_value_get_string(const Value *value);

extern Type pvm_get_value_type(const Value *value);

/**
 * Releases a VMValue and its resources
 * @param value Pointer to VMValue reference (set to NULL after freeing)
 * @note Safe to call with NULL or already freed values
 * @warning Dangling pointer risk if caller retains references
 */
extern void pvm_free_value(Value **value);


extern void pvm_free_values(const Value *value[], uint64_t length);


/**
 * Create a new PixtronVM instance
 *
 * @return Pointer to the newly created PixtronVM instance
 * @note The VM takes ownership of the buffer and will free it when destroyed
 */
extern VM *pvm_init(const char *klass_path);

/**
 * Execute the loaded bytecode in the virtual machine
 * @param vm Pointer to the PixtronVM instance to execute
 * @note This will run the VM until completion or until an error occurs
 */
extern Value *pvm_launch(const VM *vm, const char *klass_name, const char *method_name, uint16_t argv,
                         const Value *args[]);

extern void pvm_launch_main(const VM *vm, const char *klass_name);

/**
 * Destroy a PixtronVM instance and release all its resources
 * @param ref Reference to the PixtronVM instance to destroy
 * @note This will also free the associated bytecode buffer
 */
extern void pvm_destroy(VM **vm);


#endif //VM_H
