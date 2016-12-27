//
// Created by caw on 11/29/16.
//

#include "fib_merged_filter.h"
#include "bloom.h"
#include "siphasher.h"

struct fib_merged_filter {
    int N;
    int m;
    int ln2m;
    int k;

    Bitmap **filters;
    SipHasher *hasher;
    Hasher *mainHasher;
    PARCBuffer **keys;
};

void
fibMergedFilter_Destroy(FIBMergedFilter **filterP)
{
    FIBMergedFilter *filter = *filterP;

    for (int i = 0; i < filter->k; i++) {
        parcBuffer_Release(&filter->keys[i]);
    }
    free(filter->keys);

    for (int i = 0; i < filter->N; i++) {
        bitmap_Destroy(&filter->filters[i]);
    }
    free(filter->filters);

    hasher_Destroy(&filter->mainHasher);

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

        filter->filters = (Bitmap **) malloc(sizeof(Bitmap *) * N);
        for (int i = 0; i < N; i++) {
            filter->filters[i] = bitmap_Create(m);
        }

        filter->keys = (PARCBuffer **) malloc(sizeof(PARCBuffer **) * k);
        for (int i = 0; i < k; i++) {
            filter->keys[i] = parcBuffer_Allocate(SIPHASH_KEY_LENGTH);
            memset(parcBuffer_Overlay(filter->keys[i], 0), 0, SIPHASH_KEY_LENGTH);
            parcBuffer_PutUint32(filter->keys[i], k);
            parcBuffer_Flip(filter->keys[i]);
        }

        filter->hasher = siphasher_CreateWithKeys(filter->k, filter->keys);
        filter->mainHasher = hasher_Create(filter->hasher, SipHashAsHasher);
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

bool
fibMergedFilter_Insert(FIBMergedFilter *filter, Name *name, Bitmap *vector)
{
    Name *newName = name;
    if (!name_IsHashed(name)) {
        newName = name_Hash(name, filter->mainHasher, 8);
    }

    PARCBuffer *value = name_GetWireFormat(newName, name_GetSegmentCount(newName));
    Bitmap *columns = _hashedNameToVector(filter, value);
    parcBuffer_Release(&value);

    for (int r = 0; r < filter->N; r++) {
        if (bitmap_Get(vector, r)) {
            for (int c = 0; c < filter->N; c++) {
                if (bitmap_Get(columns, c)) { // if the hash pointed us to this column
                    bitmap_Set(filter->filters[r], c);
                }
            }
        }
    }

    if (!name_IsHashed(name)) {
        name_Destroy(&newName);
    }
    bitmap_Destroy(&columns);

    return true;
}

Bitmap *
fibMergedFilter_LPM(FIBMergedFilter *filter, Name *name)
{
    Name *newName = name;
    if (!name_IsHashed(name)) {
        newName = name_Hash(name, filter->mainHasher, 8);
    }

    // We still have to do LPM starting from the back
    for (int p = name_GetSegmentCount(name); p > 0; p--) {
        PARCBuffer *value = name_GetWireFormat(newName, p);
        Bitmap *columns = _hashedNameToVector(filter, value);
        parcBuffer_Release(&value);

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

        if (set) {
            if (!name_IsHashed(name)) {
                name_Destroy(&newName);
            }
            return output;
        }
    }

    if (!name_IsHashed(name)) {
        name_Destroy(&newName);
    }

    return NULL;
}


FIBInterface *MergedFilterFIBAsFIB = &(FIBInterface) {
    .LPM = (Bitmap *(*)(void *instance, const Name *ccnxName)) fibMergedFilter_LPM,
    .Insert = (bool (*)(void *instance, const Name *ccnxName, Bitmap *vector)) fibMergedFilter_Insert,
    .Destroy = (void (*)(void **instance)) fibMergedFilter_Destroy,
};
