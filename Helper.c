#include "Helper.h"
#include <glib.h>

extern inline int32_t PixtronVM_GetCStyleStrLength(const char *str) {
    g_assert(str != NULL);
    const size_t len = strlen(str);
    return (int32_t) len + 1;
}
