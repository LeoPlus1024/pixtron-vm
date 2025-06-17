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
extern inline Klass *PixtronVM_GetKlass(const PixtronVM *vm, const gchar *klassName, GError **error);


extern inline Method *PixtronVM_GetKlassMethod(const Klass *klass, const gchar *name);

#endif //KLASS_H
