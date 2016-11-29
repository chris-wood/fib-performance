//
// Created by caw on 11/23/16.
//

#include "hasher.h"
#include "siphash24.h"

#include <parc/algol/parc_Memory.h>

struct hasher {
    void *dummy;
};

Hasher *
hasher_Create()
{
    Hasher *hasher = parcMemory_Allocate(sizeof(Hasher));
    if (hasher != NULL) {

    }
    return hasher;
}

void
hasher_Destroy(Hasher **hasherP)
{
//    Hasher *hasher = *hasherP;

    parcMemory_Deallocate(hasherP);
    *hasherP = NULL;
}

PARCBuffer *
hasher_Hash(Hasher *hasher, PARCBuffer *input, PARCBuffer *key)
{
    PARCBuffer *hashOutput = parcBuffer_Allocate(SIPHASH_HASH_LENGTH);
    siphash(parcBuffer_Overlay(hashOutput, 0), parcBuffer_Overlay(input, 0),
            parcBuffer_Remaining(input), parcBuffer_Overlay(key, 0));
    return hashOutput;
}

PARCBuffer *
hasher_HashArray(Hasher *hasher, size_t length, uint8_t input[length])
{
    PARCBuffer *hashOutput = parcBuffer_Allocate(SIPHASH_HASH_LENGTH);
    PARCBuffer *key = parcBuffer_AllocateCString("1234123412341234");
    siphash(parcBuffer_Overlay(hashOutput, 0), input,
            length, parcBuffer_Overlay(key, 0));
    parcBuffer_Release(&key);
    return hashOutput;
}

PARCBitVector *
hasher_HashToVector(Hasher *hasher, PARCBuffer *input, int range, int numKeys, PARCBuffer **keys)
{
    PARCBitVector *vector = parcBitVector_Create();

    for (int i = 0; i < numKeys; i++) {
        PARCBuffer *hashOutput = hasher_Hash(hasher, input, keys[i]);
        size_t index = parcBuffer_GetUint64(hashOutput) % range;
        parcBitVector_Set(vector, index);
        parcBuffer_Release(&hashOutput);
    }

    return vector;
}