#ifndef PIXOTRONVM_H
#define PIXOTRONVM_H
#include "Type.h"

/**
 * Create a new PixtronVM instance
 * @param buffer Pointer to the bytecode buffer containing the program to execute
 * @param length Length of the bytecode buffer in bytes
 * @return Pointer to the newly created PixtronVM instance
 * @note The VM takes ownership of the buffer and will free it when destroyed
 */
extern PixtronVMPtr PixtronVM_create(uint8_t *buffer, uint32_t length);

/**
 * Execute the loaded bytecode in the virtual machine
 * @param vm Pointer to the PixtronVM instance to execute
 * @note This will run the VM until completion or until an error occurs
 */
extern void PixtronVM_exec(PixtronVMPtr vm);

/**
 * Destroy a PixtronVM instance and release all its resources
 * @param ref Reference to the PixtronVM instance to destroy
 * @note This will also free the associated bytecode buffer
 */
extern void PixtronVM_destroy(PixtronVMRef ref);

#endif //PIXOTRONVM_H
