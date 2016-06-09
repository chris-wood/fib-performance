#include "random.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

PARCBuffer *
random_Bytes(PARCBuffer *buffer)
{
    FILE *f = fopen("/dev/urandom", "r");
    if (f != NULL) {
        uint8_t *overlay = parcBuffer_Overlay(buffer, 0);
        size_t length = parcBuffer_Remaining(buffer);
        int numRead = fread(overlay, sizeof(uint8_t), length, f);
        if (numRead != length) {
            fprintf(stderr, "Failed to read %zu bytes, got %d instead", length, numRead);
        }
        fclose(f);
    }

    return buffer;
}
