#ifndef PIXOTRONVM_H
#define PIXOTRONVM_H
#include "Type.h"

/**
 * Create a new PixtronVM instance
 *
 * @return Pointer to the newly created PixtronVM instance
 * @note The VM takes ownership of the buffer and will free it when destroyed
 */
extern PixtronVM *PixtronVM_create(const gchar *workDir);

/**
 * Execute the loaded bytecode in the virtual machine
 * @param vm Pointer to the PixtronVM instance to execute
 * @note This will run the VM until completion or until an error occurs
 */
extern void PixtronVM_exec(PixtronVM *vm, const gchar *clazz, const gchar *func);

/**
 * Destroy a PixtronVM instance and release all its resources
 * @param ref Reference to the PixtronVM instance to destroy
 * @note This will also free the associated bytecode buffer
 */
extern void PixtronVM_destroy(PixtronVMRef ref);

#endif //PIXOTRONVM_H
