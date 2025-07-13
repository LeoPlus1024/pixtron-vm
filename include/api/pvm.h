#ifndef VM_H
#define VM_H
#include <stdbool.h>
#include <stdint.h>

// 类型标记（操作数中使用）
typedef enum:uint16_t {
    TYPE_BYTE,
    TYPE_SHORT,
    TYPE_INT,
    TYPE_LONG,
    TYPE_DOUBLE,
    TYPE_BOOL,
    TYPE_HANDLE,
    TYPE_VOID,
    TYPE_STRING,
    TYPE_ARRAY,
    TYPE_OBJECT,
} Type;


static const uint8_t TYPE_SIZE[] = {
    [TYPE_BYTE] = 1,
    [TYPE_SHORT] = 2,
    [TYPE_INT] = 4,
    [TYPE_LONG] = 8,
    [TYPE_DOUBLE] = 8,
    [TYPE_BOOL] = 1,
    [TYPE_HANDLE] = 8,
    [TYPE_ARRAY] = 8,
    [TYPE_OBJECT] = 8,
};

static const char *const TYPE_NAME[] = {
    [TYPE_BYTE] = "byte",
    [TYPE_SHORT] = "short",
    [TYPE_INT] = "int",
    [TYPE_LONG] = "long",
    [TYPE_DOUBLE] = "double",
    [TYPE_BOOL] = "bool",
    [TYPE_OBJECT] = "object",
    [TYPE_VOID] = "void",
    [TYPE_STRING] = "string",
    [TYPE_ARRAY] = "array",
    [TYPE_HANDLE] = "handle"
};

#define BOOL_TRUE  (1)
#define BOOL_FALSE  (0)

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

/**
 * Retrieves a copy of the string from a PVM Value object.
 *
 * This function creates and returns a newly allocated C-style string (char*)
 * containing a copy of the data from the specified Value object. The caller
 * OWNS the returned string and MUST free it using free() to avoid memory leaks.
 *
 * @param value Pointer to the Value object. Must be a string-type Value and
 *              non-NULL, otherwise returns NULL.
 *
 * @return A newly allocated mutable char* buffer containing the string copy.
 *         Returns NULL if:
 *         - The input Value is not a string type
 *         - The Value object is NULL
 *         - Memory allocation fails
 *
 * @note This is a deep copy operation. The returned string is independent
 *       of the VM's internal data. Modifications to the VM's original string
 *       will not affect the returned copy.
 *
 * @warning The returned pointer MUST be explicitly freed by the caller.
 *          Memory management responsibility is TRANSFERRED to the caller.
 */
extern char *pvm_value_get_string(const Value *value);


/**
 * Securely deallocates a PVM-allocated string and nullifies the pointer.
 *
 * This function safely releases the memory of a string obtained through
 * pvm_value_get_string() using a double-pointer strategy for robust memory management.
 *
 * @param str Address of the char* pointer holding the string.
 *               - The function will SAFELY HANDLE these cases:
 *                 - If *str_ptr is NULL: No operation is performed (ISO C11 free() compliant)
 *                 - If str_ptr itself is NULL: Triggers assertion in debug builds
 *
 * @post After execution:
 *       - The memory block is returned to the allocator
 *       - *str_ptr is set to NULL to prevent dangling pointers
 *       - Subsequent calls become safe no-ops
 *
 * @note Implementation guarantees:
 *       - Thread-safe atomic operation when built with CONFIG_THREAD_SAFE
 *       - Memory boundary checks in debug builds (via PVMMEM_SAFE_FREE)
 *       - Zero-overhead in release builds through macro optimization
 *
 * @warning Special handling requirements:
 *          - MUST ONLY be used with strings from pvm_value_get_string()
 *          - Passing pointers from malloc() directly causes undefined behavior
 *          - Always call as pvm_free_string(&your_str_ptr)
 *
 * @sample_usage
 * char **str = pvm_value_get_string(value);
 * pvm_free_string(str); // Automatic cleanup and invalidation
 */
extern void pvm_free_string(char **str);

/**
 * Retrieves the concrete type identifier of a PVM Value object.
 *
 * This function enables explicit type-driven polymorphism by exposing the
 * runtime type tag of a Value. All polymorphic behavior requires manual
 * type checking and dispatch via this API.
 *
 * @param value Pointer to the Value object to inspect.
 *            - Debug builds: Triggers assertion failure if NULL
 *            - Release builds: Returns TYPE_VOID for NULL input
 *            - Caller must ensure Value validity during inspection
 *
 * @return Fundamental Type enum value. TYPE_OBJECT is never returned -
 *         instead returns the specific concrete type (e.g. TYPE_STRING).
 *
 * @note Critical type semantics:
 *       1. TYPE_REF:
 *          - Opaque wrapper for native C pointers (e.g., FILE*, malloc buffers)
 *          - Managed through explicit destructor registration
 *          - Bypasses VM memory safety guarantees
 *
 *       2. TYPE_OBJECT:
 *          - Unified container type for passing VM objects to methods
 *          - All instance types (strings/arrays/userdata) report their
 *            concrete type through this function
 *          - No inheritance hierarchy - all types are terminal
 *
 * @sample_usage
 * void handle_value(Value* val) {
 *     switch(pvm_get_value_type(val)) {
 *         case TYPE_STRING: {
 *             const char* s = pvm_value_unwrap_string(val);
 *             printf("String: %s\n", s);
 *             break;
 *         }
 *         case TYPE_REF: {
 *             void* ptr = pvm_ref_unwrap(val);
 *             process_native_resource(ptr);
 *             break;
 *         }
 *         default:
 *             pvm_throw_error("Unsupported type");
 *     }
 * }
 *
 * @warning Polymorphism implementation pattern:
 *          - TYPE_OBJECT serves only as method parameter type
 *          - No automatic dispatch - manual switch/if chains required
 *          - Type checking MUST precede value unwrapping
 *
 * @assertion Fails catastrophically in debug builds for:
 *            - NULL input values
 *            - Corrupted type markers
 */
extern Type pvm_get_value_type(const Value *value);


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
extern Value *pvm_launch(const VM *vm, const char *klass_name, const char *method_name, uint16_t argc,
                         const Value *argv[]);

/**
 * Invokes the parameterless main function within a PVM namespace.
 *
 * This function triggers execution of the zero-argument main() function
 * located in the specified namespace. The VM will run the function to
 * completion or until an execution error occurs.
 *
 * @param vm Valid pointer to an initialized VM instance.
 *
 * @param klass_name Klass name
 *
 * @throws PVM_ENTRYPOINT_ERROR under these conditions:
 *         - Namespace not found (PVM_NAMESPACE_MISSING)
 *         - No main() function in namespace (PVM_NO_MAIN_ENTRY)
 *         - main() has parameters or return value (PVM_SIGNATURE_MISMATCH)
 *         - VM memory allocation failure (PVM_ALLOC_FAILURE)
 *
 * @note Critical implementation details:
 *       1. The main() function has strict signature requirements:
 *          - Zero parameters
 *          - void return type
 *       2. Namespace resolution is case-sensitive
 *       3. Function lookup occurs at invocation time
 */
extern void pvm_launch_main(const VM *vm, const char *klass_name);


/**
 * Destroy a PixtronVM instance and release all its resources
 * @param vm {@link VM} instance
 * @param ref Reference to the PixtronVM instance to destroy
 * @note This will also free the associated bytecode buffer
 */
extern void pvm_destroy(VM **vm);


extern Value *pvm_call_vm_method(const char *klass_name, const char *method_name, ...);


#endif //VM_H
