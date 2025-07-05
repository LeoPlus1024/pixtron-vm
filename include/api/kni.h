#ifndef KNI_H
#define KNI_H
#include <stdint.h>
#include "vm.h"

typedef struct _VMValue KniValue;
typedef struct _RuntimeContext RuntimeContext;

typedef struct {
    uint64_t len;
    char *buf;
} KniString;

extern Type pvm_kni_get_type(const KniValue *value);

extern int8_t pvm_kni_get_byte(const KniValue *value, uint16_t index);

extern int16_t pvm_kni_get_short(const KniValue *value, uint16_t index);

extern int32_t pvm_kni_get_int(const KniValue *value, uint16_t index);

extern int64_t pvm_kni_get_long(const KniValue *value, uint16_t index);

extern double pvm_kni_get_double(const KniValue *value, uint16_t index);

extern KniString *pvm_kni_get_str(const KniValue *value, uint16_t index);

extern KniValue *pvm_kni_get_object(const KniValue *value, uint16_t index);

extern void pvm_kni_release_str(KniValue **string);

#endif //KNI_H
