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
    PARCBuffer ***keys;
};

BF *
bloom_Create(int m, int k)
{
    BF *bf = (BF *) malloc(sizeof(BF));
    if (bf != NULL) {
        bf->m = m;
        bf->k = k;

        // m bits in the bucket
        bf->bucketSize = m * 8;
        bf->numBuckets = bf->bucketSize / SIPHASH_HASH_LENGTH;
        bf->array = parcBitVector_Create();

        bf->keys = parcMemory_Allocate(sizeof(PARCBuffer **) * k);
        for (int i = 0; i < k; i++) {
            bf->keys[i] = parcMemory_Allocate(sizeof(PARCBuffer *) * bf->numBuckets);
            for (int b = 0; b < bf->numBuckets; bf++) {
                bf->keys[i][b] = parcBuffer_Allocate(SIPHASH_KEY_LENGTH);
                parcBuffer_PutUint32(bf->keys[i][b], k);
                parcBuffer_Flip(bf->keys[i][b]);
            }
        }

    }
    return bf;
}

void 
bloom_Delete(BF **bfP)
{
    BF *bf = *bfP;

    parcBitVector_Release(&bf->array);
    for (int i = 0; i < bf->k; i++) {
        for (int b = 0; b < bf->numBuckets; b++) {
            parcBuffer_Release(&bf->keys[i][b]);
        }
        parcMemory_Deallocate(&bf->keys[i]);
    }
    parcMemory_Deallocate(&bf->keys);

    *bfP = NULL;
}

// XXX: incorrect
// XXX: for each k, compute k-key hash and use output % num bits as the index, and then set that index
static PARCBitVector *
_hashInput(BF *filter, PARCBuffer *value)
{
    PARCBitVector *vector = parcBitVector_Create();
    for (int i = 0; i < filter->k; i++) {

        // Build up the hash vector
        PARCBuffer *output = parcBuffer_Allocate(filter->bucketSize);
        for (int b = 0; b < filter->numBuckets; b++) {
            PARCBuffer *hashOutput = parcBuffer_Allocate(SIPHASH_HASH_LENGTH);
            siphash(parcBuffer_Overlay(hashOutput, 0), parcBuffer_Overlay(value, 0),
                    parcBuffer_Remaining(value), parcBuffer_Overlay(filter->keys[i][b], 0));
            parcBuffer_PutBuffer(output, hashOutput);
        }
        parcBuffer_Flip(output);

        // Set the right bits in the output vector
        for (int blockIndex = 0; blockIndex < filter->bucketSize; blockIndex++) {
            uint8_t block = parcBuffer_GetAtIndex(output, blockIndex);
            for (int bit = 0; bit < 8; bit++) {
                int set = (block >> (7 - bit)) & 1;
                if (set > 0) {
                    parcBitVector_Set(vector, (blockIndex * 8) + bit);
                }
            }
        }

        parcBuffer_Release(&output);
    }

    return vector;
}

void
bloom_Add(BF *filter, PARCBuffer *value)
{
    PARCBitVector *hashVector = _hashInput(filter, value);
    parcBitVector_SetVector(filter->array, hashVector);
}

bool 
bloom_Test(BF *filter, PARCBuffer *value)
{
    PARCBitVector *hashVector = _hashInput(filter, value);
    int index = -1;
    bool set = false;
    while ((index  = parcBitVector_NextBitSet(hashVector, index )) != -1) {
        if (parcBitVector_Get(filter->array, (size_t) index) == 0) {
            return false;
        }
    }
    return true;
}
