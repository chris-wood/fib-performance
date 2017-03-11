//
// Created by caw on 11/23/16.
//

#include "hasher.h"
#include "siphash24.h"

#include <parc/algol/parc_Memory.h>

struct hasher {
    void *instance;
    HasherInterface *interface;
};

Hasher *
hasher_Create(void *instance, HasherInterface *interface)
{
    Hasher *hasher = (Hasher *) malloc(sizeof(Hasher));
    if (hasher != NULL) {
        hasher->instance = instance;
        hasher->interface = interface;
    }
    return hasher;
}

void
hasher_Destroy(Hasher **hasherP)
{
    Hasher *hasher = *hasherP;
    hasher->interface->Destroy(&hasher->instance);
    free(hasher);
    *hasherP = NULL;
}

PARCBuffer *
hasher_Hash(Hasher *hasher, PARCBuffer *input)
{
    return hasher->interface->Hash(hasher->instance, input);
}

PARCBuffer *
hasher_HashArray(Hasher *hasher, size_t length, uint8_t input[length])
{
    return hasher->interface->HashArray(hasher->instance, length, input);
}

Bitmap *
hasher_HashToVector(Hasher *hasher, PARCBuffer *input, int range)
{
    return hasher->interface->HashToVector(hasher->instance, input, range);
}

Bitmap *
hasher_HashArrayToVector(Hasher *hasher, size_t length, uint8_t input[length], int range)
{
    return hasher->interface->HashArrayToVector(hasher, length, input, range);
}

PARCBuffer *
hasher_HashTruncated(Hasher *hasher, PARCBuffer *input, int limit)
{
    PARCBuffer *hash = hasher_Hash(hasher, input);
    parcBuffer_SetLimit(hash, limit);
    return hash;
}