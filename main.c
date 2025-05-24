#include <Opncode.h>
#include <stdio.h>

#include "PixotronVM.h"
#include "Type.h"

int main(void) {
    FILE *file = fopen("/home/leo/CLionProjects/PixotronVM/test.bin", "r");
    if (file == NULL) {
        printf("Error opening file\n");
        return -1;
    }
    int i;
    int size = 0;
    uint8_t buffer[1024];
    while ((i = getc(file)) != EOF) {
        buffer[size++] = (uint8_t) i;
    }
    fclose(file);

    PixtronVMPtr pixtron = PixtronVM_create(buffer, size);
    PixtronVM_exec(pixtron);
    return 0;
}
