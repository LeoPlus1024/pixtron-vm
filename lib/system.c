#include <time.h>
#include <stdio.h>

#include "itype.h"


int64_t currentTimeNano() {
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        return -1;
    }
    return ts.tv_sec * 1000000000LL + ts.tv_nsec;;
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
