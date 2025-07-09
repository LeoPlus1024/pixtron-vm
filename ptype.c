#include "ptype.h"

#include "pmem.h"
#include "pstr.h"

extern inline char *pvm_get_symbol_value(const Symbol *symbol) {
    const PStr *pstr = symbol->name;
    return pvm_string_get_data(pstr);
}


extern inline bool pvm_symbol_equal(const Symbol *a, const Symbol *b) {
    if (a == b || a->index == b->index) {
        return true;
    }
    return a->name == b->name;
}

extern inline bool pvm_symbol_cmp_cstr(const Symbol *symbol, const char *str) {
    const PStr *pstr = symbol->name;
    const char *cstr = pvm_string_get_data(pstr);
    return strcmp(cstr, str) == 0;
}

extern inline Symbol *pvm_symbol_dup(const Symbol *symbol) {
    Symbol *nsymbol = pvm_mem_calloc(sizeof(Symbol));
    memcpy(nsymbol, symbol, sizeof(Symbol));
    return nsymbol;
}
