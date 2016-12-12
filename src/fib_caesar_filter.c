#include "fib_caesar_filter.h"

#include "map.h"
#include "bloom.h"
#include "prefix_bloom.h"

#include <parc/algol/parc_SafeMemory.h>

struct fib_caesar_filter {
    PrefixBloomFilter *pbf;
    int numPorts;
    BloomFilter **portFilters;
};

void
fibCaesarFilter_Destroy(FIBCaesarFilter **fibP)
{
    FIBCaesarFilter *fib = *fibP;

    prefixBloomFilter_Destroy(&fib->pbf);
    for (int i = 0; i < fib->numPorts; i++) {
        bloom_Destroy(&fib->portFilters[i]);
    }
    free(fib->portFilters);

    free(fib);
    *fibP = NULL;
}

FIBCaesarFilter *
fibCaesarFilter_Create(int numPorts, int b, int m, int k)
{
    FIBCaesarFilter *fib = (FIBCaesarFilter *) malloc(sizeof(FIBCaesarFilter));
    if (fib != NULL) {
        fib->pbf = prefixBloomFilter_Create(b, m, k);
        fib->numPorts = numPorts;
        fib->portFilters = (BloomFilter **) malloc(numPorts * sizeof(BloomFilter *));
        for (int i = 0; i < numPorts; i++) {
            fib->portFilters[i] = bloom_Create(m, k);
        }
    }
    return fib;
}

Bitmap *
fibCaesarFilter_LPM(FIBCaesarFilter *fib, const Name *name)
{
    int numMatches = prefixBloomFilter_LPM(fib->pbf, name);
    if (numMatches >= 0) {
        Bitmap *vector = bitmap_Create(fib->numPorts);
        PARCBuffer *key = name_GetWireFormat(name, numMatches);
        for (int i = 0; i < fib->numPorts; i++) {
            bool set = false;
            if (name_IsHashed(name)) {
                set = bloom_TestHashed(fib->portFilters[i], key);
            } else {
                set = bloom_Test(fib->portFilters[i], key);
            }
            if (set) {
                bitmap_Set(vector, i);
            }
        }
        parcBuffer_Release(&key);
        return vector;
    }
    return NULL;
}

bool
fibCaesarFilter_Insert(FIBCaesarFilter *fib, const Name *name, Bitmap *vector)
{
    int numSegments = name_GetSegmentCount(name);
    PARCBuffer *key = name_GetWireFormat(name, numSegments);

    prefixBloomFilter_Add(fib->pbf, name);

    for (int i = 0; i < fib->numPorts; i++) {
        if (bitmap_Get(vector, i)) {
            if (name_IsHashed(name)) {
                bloom_AddHashed(fib->portFilters[i], key);
            } else {
                bloom_Add(fib->portFilters[i], key);
            }
        }
    }
    parcBuffer_Release(&key);

    return true;
}

FIBInterface *CaesarFilterFIBAsFIB = &(FIBInterface) {
        .LPM = (Bitmap *(*)(void *instance, const Name *ccnxName)) fibCaesarFilter_LPM,
        .Insert = (bool (*)(void *instance, const Name *ccnxName, Bitmap *vector)) fibCaesarFilter_Insert,
        .Destroy = (void (*)(void **instance)) fibCaesarFilter_Destroy,
};
