#include <stdlib.h>

#include "bloom.h"
#include "siphash24.h"
#include "hasher.h"

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_BitVector.h>

struct bloom_filter {
    int m;
    int k;
    int bucketSize;
    int numBuckets;

    Hasher *hasher;
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
        bf->hasher = hasher_Create();

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
    hasher_Destroy(&bf->hasher);
    for (int i = 0; i < bf->k; i++) {
        parcBuffer_Release(&bf->keys[i]);
    }
    parcMemory_Deallocate(&bf->keys);

    *bfP = NULL;
}

void
bloom_Add(BloomFilter *filter, PARCBuffer *value)
{
    PARCBitVector *hashVector = hasher_HashToVector(filter->hasher, value, filter->m, filter->k, filter->keys);
    parcBitVector_SetVector(filter->array, hashVector);
    parcBitVector_Release(&hashVector);
}

bool 
bloom_Test(BloomFilter *filter, PARCBuffer *value)
{
    PARCBitVector *hashVector = hasher_HashToVector(filter->hasher, value, filter->m, filter->k, filter->keys);
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
