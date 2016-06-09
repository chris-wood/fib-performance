#include "random.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

uint8_t *
random_Bytes(int n)
{
    FILE *f = fopen("/dev/urandom", "r");
    uint8_t *bytes = NULL;
    if (f != NULL) {
        bytes = (uint8_t *) malloc(n);
        int numRead = fread(bytes, sizeof(uint8_t), n, f);
        if (numRead != n) {
            fprintf(stderr, "Failed to read %d bytes, got %d instead", n, numRead);
            free(bytes);
        }
        fclose(f);
    }
    return bytes;
}
