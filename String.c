#include "String.h"

#include "Memory.h"
#include "Object.h"

static void PixtronVM_StringDestroy(const String *str) {
    char *ptr = str->str;
    if (ptr == NULL) {
        return;
    }
    PixotronVM_free(TO_REF(ptr));
}

extern inline String *PixtronVM_StringNew0(const char *str, const uint32_t len, const uint64_t initRefCount) {
    g_assert(str!=NULL);
    String *text = PixtronVM_NewObject(sizeof(String), (ObjectDestructor) PixtronVM_StringDestroy, initRefCount);
    char *cc = PixotronVM_calloc(len);
    memcpy(cc, str, len);
    text->len = len;
    text->str = cc;
    return text;
}

extern String *PixtronVM_StringCheckPool(const PixtronVM *vm, const String *str) {
    GHashTable *table = vm->strConstantPool;
    if (g_hash_table_contains(table, str)) {
        return g_hash_table_lookup(table, str);
    }
    String *text = PixtronVM_StringNew0(str->str, str->len, 0xFFFF);
    g_hash_table_insert(table, text, text);
    return text;
}

extern bool PixtronVM_StringEqual(const String *a, const String *b) {
    return a == b;
}

extern uint32_t PixtronVM_StringHash(const String *str) {
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

extern String *PixtronVM_StringNew(const char *str, const uint32_t len) {
    return PixtronVM_StringNew0(str, len, 1);
}
