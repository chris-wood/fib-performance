#include <stdlib.h>

#include "bloom.h"
#include "siphash24.h"

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_BitVector.h>

struct bloom_filter {
    int m;
    int k;
    int bucketSize;
    int numBuckets;

    PARCBitVector *array;
    PARCBuffer **keys;
};

BloomFilter *
bloom_Create(int m, int k)
{
    BloomFilter *bf = (BloomFilter *) malloc(sizeof(BloomFilter));
    if (bf != NULL) {
        bf->m = m;
        bf->k = k;

        // m bits in the bucket
        bf->bucketSize = m * 8;
        bf->numBuckets = bf->bucketSize / SIPHASH_HASH_LENGTH;
        bf->array = parcBitVector_Create();

        bf->keys = parcMemory_Allocate(sizeof(PARCBuffer **) * k);
        for (int i = 0; i < k; i++) {
            bf->keys[i] = parcBuffer_Allocate(SIPHASH_KEY_LENGTH);
            parcBuffer_PutUint32(bf->keys[i], k);
            parcBuffer_Flip(bf->keys[i]);
        }

    }
    return bf;
}

void
bloom_Destroy(BloomFilter **bfP)
{
    BloomFilter *bf = *bfP;

    parcBitVector_Release(&bf->array);
    for (int i = 0; i < bf->k; i++) {
        parcBuffer_Release(&bf->keys[i]);
    }
    parcMemory_Deallocate(&bf->keys);

    *bfP = NULL;
}

static PARCBitVector *
_hashInput(BloomFilter *filter, PARCBuffer *value)
{
    PARCBitVector *vector = parcBitVector_Create();
    for (int i = 0; i < filter->k; i++) {
        PARCBuffer *hashOutput = parcBuffer_Allocate(SIPHASH_HASH_LENGTH);
        siphash(parcBuffer_Overlay(hashOutput, 0), parcBuffer_Overlay(value, 0),
                parcBuffer_Remaining(value), parcBuffer_Overlay(filter->keys[i], 0));
        size_t index = parcBuffer_GetUint64(hashOutput) % filter->m;
        parcBitVector_Set(vector, index);
        parcBuffer_Release(&hashOutput);
    }

    return vector;
}

void
bloom_Add(BloomFilter *filter, PARCBuffer *value)
{
    PARCBitVector *hashVector = _hashInput(filter, value);
    parcBitVector_SetVector(filter->array, hashVector);
    parcBitVector_Release(&hashVector);
}

bool 
bloom_Test(BloomFilter *filter, PARCBuffer *value)
{
    PARCBitVector *hashVector = _hashInput(filter, value);
    int index = parcBitVector_NextBitSet(hashVector, 0);

    if (index == -1) {
        return false;
    }

    while (index != -1) {
        int bitValue = parcBitVector_Get(filter->array, (size_t) index);
        if (bitValue != 1) {
            parcBitVector_Release(&hashVector);
            return false;
        }
        index = parcBitVector_NextBitSet(hashVector, index + 1);
    }

    parcBitVector_Release(&hashVector);
    return true;
}
