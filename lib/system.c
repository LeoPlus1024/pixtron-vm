#include <time.h>
#include <stdio.h>

#include "itype.h"
#include "memory.h"

int64_t currentTimeNano() {
    struct timespec ts;
    if (timespec_get(&ts, TIME_UTC) != TIME_UTC) {
        return -1;
    }
    return ts.tv_nsec;
}

void printNanoDiff(const int64_t base, const int64_t now) {
    printf("%ld ns\n", (now - base));
}


void println(char *str) {
    if (str == NULL) {
        printf("null\n");
        return;
    }
    printf("%s\n", str);
}
