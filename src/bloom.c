#include <stdlib.h>

#include "bloom.h"
#include "siphasher.h"

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_BitVector.h>

struct bloom_filter {
    int m;
    int ln2m;
    int k;
    int bucketSize;
    int numBuckets;

    SipHasher *hasher;
    PARCBitVector *array;
    PARCBuffer **keys;
};

static int
_log2(int x) {
    int n = x;
    int bits = 0;
    while (n > 0) {
        n >>= 1;
        bits++;
    }
    return bits;
}

BloomFilter *
bloom_Create(int m, int k)
{
    BloomFilter *bf = (BloomFilter *) malloc(sizeof(BloomFilter));
    if (bf != NULL) {
        bf->m = m;
        bf->ln2m = _log2(m);
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

        bf->hasher = siphasher_CreateWithKeys(k, bf->keys);

    }
    return bf;
}

void
bloom_Destroy(BloomFilter **bfP)
{
    BloomFilter *bf = *bfP;

    parcBitVector_Release(&bf->array);
    siphasher_Destroy(&bf->hasher);
    for (int i = 0; i < bf->k; i++) {
        parcBuffer_Release(&bf->keys[i]);
    }
    parcMemory_Deallocate(&bf->keys);

    *bfP = NULL;
}

void
bloom_Add(BloomFilter *filter, PARCBuffer *value)
{
    PARCBitVector *hashVector = siphasher_HashToVector(filter->hasher, value, filter->m);
    parcBitVector_SetVector(filter->array, hashVector);
    parcBitVector_Release(&hashVector);
}

bool 
bloom_Test(BloomFilter *filter, PARCBuffer *value)
{
    PARCBitVector *hashVector = siphasher_HashToVector(filter->hasher, value, filter->m);
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

void
bloom_AddHashed(BloomFilter *filter, PARCBuffer *value)
{
    int numBytesRequired = (filter->k * filter->ln2m) / 8;
    size_t inputSize = parcBuffer_Remaining(value);
    if (inputSize < numBytesRequired) {
        assertTrue(false, "Invalid bloom filter hash input -- expected at least %d bytes, got %zu", numBytesRequired, inputSize);
        return;
    }

    // |B| = input size [bytes]
    // k = number of blocks needed
    // |B| / k = number of bytes to include in each block
    int blockSize = inputSize / filter->k;
    for (int i = 0; i < filter->k; i++) {
        size_t checkSum = 0;
        for (int b = 0; b < blockSize; b++) {
            checkSum += parcBuffer_GetUint8(value);
        }
        checkSum %= filter->m;

        // Set the target bit
        parcBitVector_Set(filter->array, checkSum);
    }

    // Reset the input state
    parcBuffer_Flip(value);
}

bool
bloom_TestHashed(BloomFilter *filter, PARCBuffer *value)
{
    int numBytesRequired = (filter->k * filter->ln2m) / 8;
    size_t inputSize = parcBuffer_Remaining(value);
    if (inputSize < numBytesRequired) {
        assertTrue(false, "Invalid bloom filter hash input -- expected at least %d bytes, got %zu", numBytesRequired, inputSize);
        return false;
    }

    int blockSize = inputSize / filter->k;
    for (int i = 0; i < filter->k; i++) {
        size_t checkSum = 0;
        for (int b = 0; b < blockSize; b++) {
            checkSum += parcBuffer_GetUint8(value);
        }
        checkSum %= filter->m;

        // Query the target bit
        if (parcBitVector_Get(filter->array, checkSum) != 1) {
            parcBuffer_Flip(value);
            return false;
        }
    }

    // Reset the input state
    parcBuffer_Flip(value);

    return true;
}