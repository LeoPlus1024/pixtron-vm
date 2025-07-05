#include "pstr.h"

#include <stdio.h>

#include "memory.h"
#include "pobject.h"

static void pvm_string_destructor(const PStr *str) {
    char *ptr = str->str;
    if (ptr == NULL) {
        return;
    }
    pvm_mem_free(TO_REF(ptr));
}

extern PStr *pvm_string_const_pool_add(const PixtronVM *vm, const PStr *str) {
    GHashTable *table = vm->string_constants;
    if (g_hash_table_contains(table, str)) {
        return g_hash_table_lookup(table, str);
    }
    PStr *text = pvm_string_new(str->str, str->len);
    g_hash_table_insert(table, text, text);
    return text;
}

extern bool pvm_string_equal(const PStr *a, const PStr *b) {
    return a == b;
}

extern uint32_t pvm_string_hash(const PStr *str) {
    const char *p = str->str;
    uint32_t n = str->len;
    uint32_t h = 0;

    /* 31 bit hash function */
    while (n--) {
        h = (h << 5) - h + *p;
        p++;
    }

    return h;
}

extern PStr *pvm_string_new(const char *str, const uint32_t len) {
    g_assert(str!=NULL);
    PStr *text = pvm_object_new(sizeof(PStr), (ObjectDestructor) pvm_string_destructor, 1);
    char *cc = pvm_mem_calloc(len);
    memcpy(cc, str, len);
    text->len = len;
    text->str = cc;
    return text;
}

extern char *pvm_string_to_cstr(const PStr *str) {
    if (str == NULL || str->len == 0) {
        return NULL;
    }
    const uint32_t len = str->len + 1;
    char *c_str = pvm_mem_cpy(str->str, len);
    c_str[len] = '\0';
    return c_str;
}
