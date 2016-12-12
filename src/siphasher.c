//
// Created by Christopher Wood on 12/5/16.
//

#include "siphasher.h"
#include "siphash24.h"

struct siphasher {
    int numKeys;
    PARCBuffer **keys;
};

SipHasher *
siphasher_Create(PARCBuffer *key)
{
    SipHasher *hasher = (SipHasher *) malloc(sizeof(SipHasher));
    if (hasher != NULL) {
        hasher->keys = (PARCBuffer **) malloc(sizeof(PARCBuffer *));
        hasher->keys[0] = parcBuffer_Acquire(key);
        hasher->numKeys = 1;
    }
    return hasher;
}

SipHasher *
siphasher_CreateWithKeys(int numKeys, PARCBuffer *keys[numKeys])
{
    SipHasher *hasher = (SipHasher *) malloc(sizeof(SipHasher));
    if (hasher != NULL) {
        hasher->numKeys = numKeys;
        hasher->keys = (PARCBuffer **) malloc(numKeys * sizeof(PARCBuffer *));
        for (int i = 0; i < numKeys; i++) {
            hasher->keys[i] = parcBuffer_Acquire(keys[i]);
        }
    }
    return hasher;
}

void
siphasher_Destroy(SipHasher **hasherP)
{
    SipHasher *hasher = *hasherP;
    for (int i = 0; i < hasher->numKeys; i++) {
        parcBuffer_Release(&hasher->keys[i]);
    }
    free(hasher->keys);
    free(hasher);
    *hasherP = NULL;
}

PARCBuffer *
siphasher_Hash(SipHasher *hasher, PARCBuffer *input)
{
    PARCBuffer *hashOutput = parcBuffer_Allocate(SIPHASH_HASH_LENGTH);
    uint8_t *outputOverlay = parcBuffer_Overlay(hashOutput, 0);
    uint8_t *inputOverlay = parcBuffer_Overlay(input, 0);
    uint8_t *keyOverlay = parcBuffer_Overlay(hasher->keys[0], 0);
    size_t inputSize = parcBuffer_Remaining(input);

    siphash(outputOverlay, inputOverlay, inputSize, keyOverlay);

    return hashOutput;
}

PARCBuffer *
siphasher_HashArray(SipHasher *hasher, size_t length, uint8_t input[length])
{
    PARCBuffer *hashOutput = parcBuffer_Allocate(SIPHASH_HASH_LENGTH);
    siphash(parcBuffer_Overlay(hashOutput, 0), input,
            length, parcBuffer_Overlay(hasher->keys[0], 0));
    return hashOutput;
}

Bitmap *
siphasher_HashToVector(SipHasher *hasher, PARCBuffer *input, int range)
{
    Bitmap *vector = bitmap_Create(range);

    for (int i = 0; i < hasher->numKeys; i++) {
        PARCBuffer *hashOutput = siphasher_Hash(hasher, input);
        size_t index = parcBuffer_GetUint64(hashOutput) % range;
        bitmap_Set(vector, index);
        parcBuffer_Release(&hashOutput);
    }

    return vector;
}

Bitmap *
siphasher_HashArrayToVector(SipHasher *hasher, size_t length, uint8_t input[length], int range)
{
    Bitmap *vector = bitmap_Create(range);

    for (int i = 0; i < hasher->numKeys; i++) {
        PARCBuffer *hashOutput = siphasher_HashArray(hasher, length, input);
        size_t index = parcBuffer_GetUint64(hashOutput) % range;
        bitmap_Set(vector, index);
        parcBuffer_Release(&hashOutput);
    }

    return vector;
}

HasherInterface *SipHashAsHasher = &(HasherInterface) {
        .Hash = (PARCBuffer *(*)(void *, PARCBuffer *)) siphasher_Hash,
        .HashArray = (PARCBuffer *(*)(void *hasher, size_t length, uint8_t *input)) siphasher_HashArray,
        .HashToVector = (Bitmap *(*)(void*hasher, PARCBuffer *input, int range)) siphasher_HashToVector,
        .HashArrayToVector = (Bitmap *(*)(void*hasher, size_t length, uint8_t *input, int range)) siphasher_HashArrayToVector,
        .Destroy = (void (*)(void **instance)) siphasher_Destroy,
};