#include "kni.h"
#include <math.h>

void Math_pow(RuntimeContext *context, KniValue *result) {
    const double a = pvm_kni_get_double(context, 0);
    const double b = pvm_kni_get_double(context, 1);
    const double value = pow(a, b);
    pvm_kni_set_double(result, value);
}
