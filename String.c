//
// Created by leo on 25-6-16.
//

#include "String.h"

extern inline gint PixtronVM_GetStrFullLen(const gchar *str) {
    g_assert(str != NULL);
    const size_t len = strlen(str);
    return (gint) len + 1;
}
