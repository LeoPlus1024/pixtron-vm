#ifndef KLASS_H
#define KLASS_H
#include "ptype.h"

/**
 * Retrieves a class (Klass) by name from the virtual machine's class table.
 * If the class is not cached, loads it dynamically and handles errors.
 *
 * @param vm Pointer to the PixtronVM instance
 * @param klassName Name of the class to retrieve (case-sensitive)
 * @return Pointer to the requested Klass, or exits thread on failure
 *
 * @note Thread termination occurs if class loading fails - consider alternative
 *       error handling for production environments.
 */
extern inline Klass *pvm_get_klass(const PixtronVM *vm, const char *klassName, GError **error);


extern inline void pvm_get_klass_field(RuntimeContext *context, uint16_t index, VMValue *value);


extern inline void pvm_set_klass_field(RuntimeContext *context, uint16_t index, const VMValue *value);

extern inline void pvm_get_klass_constant(RuntimeContext *context, uint16_t index, VMValue *value);

extern inline Method *pvm_get_method(RuntimeContext *context, uint16_t index);

extern inline Method *pvm_get_method_by_name(const Klass *klass, const char *name);

extern inline void pvm_load_system_klass(const PixtronVM *vm, GError **error);

#endif //KLASS_H
