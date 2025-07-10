#include "pstr.h"

#include <assert.h>
#include <stdio.h>

#include "pmem.h"
#include "pobj.h"

static void pvm_string_destructor(const PStr *str) {
    char *ptr = str->value;
    if (ptr == NULL || str->len <= 7) {
        return;
    }
    pvm_mem_free(TO_REF(ptr));
}

extern PStr *pvm_string_intern(const PixtronVM *vm, const int32_t length, char *c_str) {
    GHashTable *table = vm->string_table;
    const PStr tmp = {
        .len = length,
        .value = c_str
    };
    PStr *pstr = g_hash_table_lookup(table, &tmp);
    if (pstr != NULL) {
        return pstr;
    }
    pstr = pvm_string_new(c_str, length);
    g_hash_table_insert(table, pstr, pstr);
    return pstr;
}

extern bool pvm_string_equal(const PStr *a, const PStr *b) {
    if (a == b) {
        return true;
    }
    if (a->len != b->len) {
        return false;
    }
    return memcmp(pvm_string_get_data(a), pvm_string_get_data(b), a->len) == 0;
}

extern uint32_t pvm_string_hash(PStr *str) {
    if (str->hash_code != 0) {
        return str->hash_code;
    }

    const char *p = pvm_string_get_data(str);

    uint32_t n = str->len;
    uint32_t h = 0;

    while (n--) {
        h = (h << 5) - h + *p;
        p++;
    }

    str->hash_code = h;
    return h;
}

extern PStr *pvm_string_new(const char *str, const uint32_t len) {
    assert(str!=NULL);
    PStr *pstr = pvm_object_new(sizeof(PStr), (ObjectDestructor) pvm_string_destructor, 1);
    pstr->len = len;
    if (len <= 7) {
        memcpy(pstr->values, str, len);
        return pstr;
    }
    char *buffer = pvm_mem_calloc(len + 1);;
    pstr->value = memcpy(buffer, str, len);
    return pstr;
}

extern inline char *pvm_string_get_data(const PStr *pstr) {
    assert(pstr!=NULL);
    if (pstr->len < 8) {
        return pstr->values;
    }
    return pstr->value;
}

extern char *pvm_string_to_cstr(const PStr *str) {
    if (str == NULL || str->len == 0) {
        return NULL;
    }
    const uint32_t len = str->len + 1;
    const char *buffer = pvm_string_get_data(str);
    char *c_str = pvm_mem_cpy(buffer, len);
    return c_str;
}
