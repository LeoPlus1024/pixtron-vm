#include "Kni.h"
#include <math.h>

void Math_pow(RuntimeContext *context, KniValue *result) {
    const double a = Kni_GetDoubleParam(context, 0);
    const double b = Kni_GetDoubleParam(context, 1);
    const double value = pow(a, b);
    Kni_SetDouble(result, value);
}
