//
// Created by caw on 11/29/16.
//

#include "merged_bloom.h"
#include "bloom.h"
#include "siphasher.h"

struct fib_merged_filter {
    int N;
    int m;
    int ln2m;
    int k;

    Bitmap **filters;
    SipHasher *hasher;
    PARCBuffer **keys;
};

void
fibMergedFilter_Destroy(FIBMergedFilter **filterP)
{
    FIBMergedFilter *filter = *filterP;

    for (int i = 0; i < filter->N; i++) {
        bloom_Destroy(&filter->filters[i]);
    }
    free(filter->filters);

    free(filter);
    *filterP = NULL;
}

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

FIBMergedFilter *
fibMergedFilter_Create(int N, int m, int k) // N and m determine the matrix dimensions
{
    FIBMergedFilter *filter = (FIBMergedFilter *) malloc(sizeof(FIBMergedFilter));
    if (filter != NULL) {
        filter->N = N; // N = m here, by design
        filter->m = m;
        filter->k = k;
        filter->ln2m = _log2(m);

        filter->filters = (BloomFilter **) malloc(sizeof(BloomFilter *) * N);
        for (int i = 0; i < N; i++) {
            filter->filters[i] = bloom_Create(m, k);
        }

        filter->keys = (PARCBuffer **) malloc(sizeof(PARCBuffer **) * k);
        for (int i = 0; i < k; i++) {
            filter->keys[i] = parcBuffer_Allocate(SIPHASH_KEY_LENGTH);
            memset(parcBuffer_Overlay(filter->keys[i], 0), 0, SIPHASH_KEY_LENGTH);
            parcBuffer_PutUint32(filter->keys[i], k);
            parcBuffer_Flip(filter->keys[i]);
        }

        filter->hasher = siphasher_CreateWithKeys(filter->k, filter->keys);
    }
    return filter;
}

static Bitmap *
_hashedNameToVector(FIBMergedFilter *filter, PARCBuffer *value)
{
    int numBytesRequired = (filter->k * filter->ln2m) / 8;
    size_t inputSize = parcBuffer_Remaining(value);
    int blockSize = inputSize / filter->k;
    uint8_t *overlay = parcBuffer_Overlay(value, 0);

    Bitmap *map = bitmap_Create(filter->N);

    for (int i = 0; i < filter->k; i++) {
        size_t checkSum = 0;
        for (int b = 0; b < blockSize; b++) {
            checkSum += overlay[(i * blockSize) + b];
        }
        checkSum %= filter->m;
        bitmap_Set(map, checkSum);
    }

    return map;
}

void
fibMergedFilter_Add(FIBMergedFilter *filter, Name *name, Bitmap *vector)
{
    PARCBuffer *value = name_GetWireFormat(name, name_GetSegmentCount(name));
    Bitmap *columns = NULL;
    if (name_IsHashed(name)) {
        Bitmap *columns = _hashedNameToVector(filter, value);
    } else {
        Bitmap *columns = siphasher_HashToVector(filter->hasher, value, filter->m);
    }

    for (int r = 0; r < filter->N; r++) {
        if (bitmap_Get(vector, r)) {
            for (int c = 0; c < filter->N; c++) {
                if (bitmap_Get(columns, c)) { // if the hash pointed us to this column
                    bitmap_Set(filter->filters[r], c);
                }
            }
        }
    }

    bitmap_Destroy(&columns);
    parcBuffer_Release(&value);
}

Bitmap *
fibMergedFilter_Lookup(FIBMergedFilter *filter, Name *name)
{
    // We still have to do LPM starting from the back...
    for (int p = name_GetSegmentCount(name); p > 0; p--) {
        PARCBuffer *value = name_GetWireFormat(name, p);
        Bitmap *columns = NULL;
        if (name_IsHashed(name)) {
            Bitmap *columns = _hashedNameToVector(filter, value);
        } else {
            Bitmap *columns = siphasher_HashToVector(filter->hasher, value, filter->m);
        }

        Bitmap *output = bitmap_Create(filter->N);
        bool set = false;

        for (int r = 0; r < filter->N; r++) {
            bool isMatch = true;
            for (int c = 0; c < filter->N; c++) {
                if (bitmap_Get(columns, c) && !bitmap_Get(filter->filters[r], c)) { // if the hash pointed us to this column
                    isMatch = false;
                    break;
                }
            }

            // All columns matched, so we must forward to this port
            if (isMatch) {
                bitmap_Set(output, r);
                set = true;
            }
        }

        bitmap_Destroy(&columns);
        parcBuffer_Release(&value);

        if (set) {
            return output);
        }
    }

    return NULL;
}
