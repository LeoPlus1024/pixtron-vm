#include "StringUtil.h"

extern inline gint PixtronVM_GetStrFullLen(const gchar *str) {
    g_assert(str != NULL);
    const size_t len = strlen(str);
    return (gint) len + 1;
}
