#include <stdlib.h>

#include "bloom.h"
#include "siphasher.h"

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_BitVector.h>

// debug
#include <stdio.h>
#include "timer.h"

struct bloom_filter {
    int m;
    int ln2m;
    int k;

    SipHasher *hasher;
    Hasher **vectorHashers;
    Bitmap *array;
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
        bf->array = bitmap_Create(m);

        bf->keys = parcMemory_Allocate(sizeof(PARCBuffer **) * k);
        for (int i = 0; i < k; i++) {
            bf->keys[i] = parcBuffer_Allocate(SIPHASH_KEY_LENGTH);
            memset(parcBuffer_Overlay(bf->keys[i], 0), 0, SIPHASH_KEY_LENGTH);
            parcBuffer_PutUint32(bf->keys[i], k);
            parcBuffer_Flip(bf->keys[i]);
        }

        bf->vectorHashers = (Hasher **) malloc(k * sizeof(Hasher *));
        for (int i = 0; i < k; i++) {
            SipHasher *hasher = siphasher_Create(bf->keys[i]);
            bf->vectorHashers[i] = hasher_Create(hasher, SipHashAsHasher);
        }

        bf->hasher = siphasher_CreateWithKeys(k, bf->keys);
    }
    return bf;
}

void
bloom_Destroy(BloomFilter **bfP)
{
    BloomFilter *bf = *bfP;

    for (int i = 0; i < bf->k; i++) {
        parcBuffer_Release(&bf->keys[i]);
        hasher_Destroy(&bf->vectorHashers[i]);
    }
    bitmap_Destroy(&bf->array);
    siphasher_Destroy(&bf->hasher);
    parcMemory_Deallocate(&bf->keys);
    free(bf->vectorHashers);

    *bfP = NULL;
}

void
bloom_AddName(BloomFilter *filter, Name *name)
{
    // compute the d hashes for the first (k = 0) hash
    Name *newName = name_Hash(name, filter->vectorHashers[0], 8);

    // compute the bit for the first hash (k = 0)
    PARCBuffer *segmentHash = name_GetWireFormat(newName, name_GetSegmentCount(name));
    size_t checkSum = 0;
    uint8_t *segmentHashOverlay = parcBuffer_Overlay(segmentHash, 0);
    for (int b = 0; b < parcBuffer_Remaining(segmentHash); b++) {
        checkSum += segmentHashOverlay[b];
    }

    // set the target bit
    checkSum %= filter->m;
    bitmap_Set(filter->array, checkSum);

    parcBuffer_Release(&segmentHash);

    // use XOR to seed out the remaining hash values for the d prefixes, for each other hash function
    for (int i = 1; i < filter->k; i++) {

        // the first component for the other hashes is always fed through the hasher
        PARCBuffer *firstComponent = name_GetWireFormat(newName, 1);
        PARCBuffer *firstHash = hasher_Hash(filter->vectorHashers[i], firstComponent);

        // the rest are derived by XORing this first segment with the d-th segment of the first hash function, i.e.,
        //    H_{i,j} = H_{i, 1} XOR H_{1, j}
        // where i = k and j = d
        PARCBuffer *segmentHash = name_XORSegment(newName, name_GetSegmentCount(name) - 1, firstHash);

        size_t checkSum = 0;
        uint8_t *segmentHashOverlay = parcBuffer_Overlay(segmentHash, 0);
        for (int b = 0; b < parcBuffer_Remaining(segmentHash); b++) {
            checkSum += segmentHashOverlay[b];
        }
        checkSum %= filter->m;

        // Finally, set the target bit.
        bitmap_Set(filter->array, checkSum);
        parcBuffer_Release(&segmentHash);

        parcBuffer_Release(&firstComponent);
        parcBuffer_Release(&firstHash);
    }

    name_Destroy(&newName);
}

static void
_freeBitMatrix(int **bitMatrix, int rows)
{
    for (int k = 0; k < rows; k++) {
        free(bitMatrix[k]);
    }
    free(bitMatrix);
}

int
bloom_TestName(BloomFilter *filter, Name *name)
{
    // compute the d hashes for the first (k = 0) hash
    Timestamp start = timerStart();
    Name *newName = name_Hash(name, filter->vectorHashers[0], 8);
    long elapsed = timerEnd(start);
    printf("Hash name: %ld\n", elapsed);

    // Allocate space for the bit matrix
    int **bitMatrix = (int **) malloc(filter->k * sizeof(int *));
    for (int k = 0; k < filter->k; k++) {
        bitMatrix[k] = (int *) malloc(name_GetSegmentCount(newName) * sizeof(int));
        for (int d = 0; d < name_GetSegmentCount(newName); d++) {
            bitMatrix[k][d] = 0;
        }
    }

    // Compute the first row of the bit matrix
    start = timerStart();
    for (int d = 0; d < name_GetSegmentCount(newName); d++) {
        PARCBuffer *segmentHash = name_GetWireFormat(newName, d + 1);
        size_t checkSum = 0;
        uint8_t *segmentHashOverlay = parcBuffer_Overlay(segmentHash, 0);
        for (int b = 0; b < parcBuffer_Remaining(segmentHash); b++) {
            checkSum += segmentHashOverlay[b];
        }
        checkSum %= filter->m;
        bitMatrix[0][d] = checkSum;
        parcBuffer_Release(&segmentHash);
    }
    elapsed = timerEnd(start);
    printf("First row: %ld\n", elapsed);

    // use XOR to seed out the remaining hash values for the d prefixes, for each other hash function
    for (int i = 1; i < filter->k; i++) {

        start = timerStart();

        // the first component for the other hashes is always fed through the hasher
        PARCBuffer *firstComponent = name_GetWireFormat(newName, 1);
        PARCBuffer *firstHash = hasher_Hash(filter->vectorHashers[i], firstComponent);

        // the rest are derived by XORing this first segment with the d-th segment of the first hash function, i.e.,
        //    H_{i,j} = H_{i, 1} XOR H_{1, j}
        // where i = k and j = d
        for (int d = 0; d < name_GetSegmentCount(newName); d++) {
            PARCBuffer *segmentHash = name_XORSegment(newName, d, firstHash);

            size_t checkSum = 0;
            uint8_t *segmentHashOverlay = parcBuffer_Overlay(segmentHash, 0);
            for (int b = 0; b < parcBuffer_Remaining(segmentHash); b++) {
                checkSum += segmentHashOverlay[b];
            }
            checkSum %= filter->m;

            // Finally, set the target bit in the matrix
            bitMatrix[i][d] = checkSum;

            parcBuffer_Release(&segmentHash);
        }

        parcBuffer_Release(&firstComponent);
        parcBuffer_Release(&firstHash);

        elapsed = timerEnd(start);
        printf("%dth vector: %ld\n", i, elapsed);
    }

    // Now do LPM on the bit matrix
    for (int d = name_GetSegmentCount(name) - 1; d >= 0; d--) {
        bool allMatch = true;
        for (int k = 0; k < filter->k; k++) {
            if (bitmap_Get(filter->array, bitMatrix[k][d]) != 1) {
                allMatch = false;
                break;
            }
        }

        if (allMatch) {
            _freeBitMatrix(bitMatrix, filter->k);
            name_Destroy(&newName);
            return d + 1;
        }
    }

    _freeBitMatrix(bitMatrix, filter->k);
    name_Destroy(&newName);
    return -1;
}

void
bloom_Add(BloomFilter *filter, PARCBuffer *value)
{
    Bitmap *hashVector = siphasher_HashToVector(filter->hasher, value, filter->m);
    bitmap_SetVector(filter->array, hashVector);
    bitmap_Destroy(&hashVector);
}

void
bloom_AddRaw(BloomFilter *filter, int length, uint8_t value[length])
{
    Bitmap *hashVector = siphasher_HashArrayToVector(filter->hasher, length, value, filter->m);
    bitmap_SetVector(filter->array, hashVector);
    bitmap_Destroy(&hashVector);
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
    uint8_t *overlay = parcBuffer_Overlay(value, 0);
    for (int i = 0; i < filter->k; i++) {
        size_t checkSum = 0;
        for (int b = 0; b < blockSize; b++) {
            checkSum += overlay[(i * blockSize) + b];
        }
        checkSum %= filter->m;

        // Set the target bit
        bitmap_Set(filter->array, checkSum);
    }
}

bool
bloom_Test(BloomFilter *filter, PARCBuffer *value)
{
    Bitmap *hashVector = siphasher_HashToVector(filter->hasher, value, filter->m);
    bool contains = bitmap_Contains(filter->array, hashVector);
    bitmap_Destroy(&hashVector);
    return contains;
}

bool
bloom_TestRaw(BloomFilter *filter, int length, uint8_t value[length])
{
    Bitmap *hashVector = siphasher_HashArrayToVector(filter->hasher, length, value, filter->m);
    bool contains = bitmap_Contains(filter->array, hashVector);
    bitmap_Destroy(&hashVector);
    return contains;
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
    uint8_t *overlay = parcBuffer_Overlay(value, 0);
    for (int i = 0; i < filter->k; i++) {
        size_t checkSum = 0;
        for (int b = 0; b < blockSize; b++) {
            checkSum += overlay[(i * blockSize) + b];
        }
        checkSum %= filter->m;

        // Query the target bit
        if (!bitmap_Get(filter->array, checkSum)) {
            return false;
        }
    }

    return true;
}
