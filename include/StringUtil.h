#ifndef STRING_UTIL_H
#define STRING_UTIL_H
#include <glib.h>
/**
 * Calculates the full buffer size needed to store a null-terminated C string.
 *
 * This includes the string's content plus the null terminator. For example:
 *   "Hello" (strlen=5) requires a buffer size of 6.
 *
 * @param str Null-terminated input string (must not be NULL)
 * @return Total buffer size required to store the string including null terminator
 *
 * @note Uses gint return type for GLib compatibility, but note size limitations:
 *       Maximum theoretical string length is G_MAXINT-1 (2^31-2 on 32-bit systems)
 *       due to signed return type. Consider gsize for pure size calculations.
 */
extern inline gint PixtronVM_GetStrFullLen(const gchar *str);
#endif //STRING_UTIL_H
