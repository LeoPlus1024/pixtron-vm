#ifndef PHANDLE_H
#define PHANDLE_H
#include "ptype.h"

typedef struct _PHandle PHandle;

typedef void *Handle;

extern inline PHandle *pvm_handle_new(Handle handle, Method *cleanup);


extern inline void *pvm_handle_get_data(const PHandle *handle);

#endif //PHANDLE_H
