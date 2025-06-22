#ifndef KLASS_H
#define KLASS_H
#include "Type.h"

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
extern inline Klass *PixtronVM_GetKlass(const PixtronVM *vm, const char *klassName, GError **error);


extern inline void PixtronVM_GetKlassFileValue(RuntimeContext *context, uint16_t index, VMValue *value);


extern inline void PixtronVM_SetKlassFileValue(RuntimeContext *context, uint16_t index, const VMValue *value);

extern inline void PixtronVM_GetKlassConstant(RuntimeContext *context, uint16_t index, VMValue *value);

extern inline Method *PixtronVM_GetKlassMethod(RuntimeContext *context, uint16_t index);

extern inline Method *PixtronVM_GetKlassMethodByName(const Klass *klass, const char *name);

extern inline void PixtronVM_InitSystemKlass(const PixtronVM *vm, GError **error);

#endif //KLASS_H
