//
// Created by Christopher Wood on 11/22/16.
//

#include <stdlib.h>

#include <parc/algol/parc_Memory.h>

#include "prefix_bloom.h"

#include "bloom.h"
#include "hasher.h"

struct prefix_bloom_filter {
    int k;
    int m;
    int b;

    BloomFilter **filterBlocks;
    Hasher *hasher;
};

PrefixBloomFilter *
prefixBloomFilter_Create(int b, int m, int k)
{
    PrefixBloomFilter *filter = parcMemory_Allocate(sizeof(PrefixBloomFilter));
    if (filter != NULL) {
        filter->b = b;
        filter->m = m;
        filter->k = k;
        filter->hasher = hasher_Create();

        filter->filterBlocks = parcMemory_Allocate(b * sizeof(PrefixBloomFilter *));
        for (int i = 0; i < b; i++) {
            filter->filterBlocks[i] = bloom_Create(m, k);
        }
    }
    return filter;
}

void
prefixBloomFilter_Destroy(PrefixBloomFilter **bfP)
{
    PrefixBloomFilter *filter = *bfP;

    for (int i = 0; i < filter->b; i++) {
        bloom_Destroy(&filter->filterBlocks[i]);
    }
    parcMemory_Deallocate(&filter->filterBlocks);
    hasher_Destroy(&filter->hasher);

    parcMemory_Deallocate(bfP);
    *bfP = NULL;
}

void
prefixBloomFilter_Add(PrefixBloomFilter *filter, Name *name)
{
    // 1. hash first segment to identify the block
    PARCBuffer *firstSegmentHash = hasher_HashArray(filter->hasher, name_GetSegmentLength(name, 0), name_GetSegmentOffset(name, 0));
    uint64_t blockIndex = parcBuffer_GetUint64(firstSegmentHash) % filter->b;

    // 2. add the k hashes to the filter
    PARCBuffer *nameValue = name_GetWireFormat(name, name_GetSegmentCount(name));
    bloom_Add(filter->filterBlocks[blockIndex], nameValue);

    parcBuffer_Release(&nameValue);
    parcBuffer_Release(&firstSegmentHash);
}

int
prefixBloomFilter_LPM(PrefixBloomFilter *filter, Name *name)
{
    // 1. hash first segment to identify the block
    PARCBuffer *firstSegmentHash = hasher_HashArray(filter->hasher, name_GetSegmentLength(name, 0), name_GetSegmentOffset(name, 0));
    uint64_t blockIndex = parcBuffer_GetUint64(firstSegmentHash) % filter->b;

    // 2. add the k hashes to the filter
    for (int count = name_GetSegmentCount(name); count > 0; count--) {
        PARCBuffer *nameValue = name_GetWireFormat(name, count);
        bool isPresent = bloom_Test(filter->filterBlocks[blockIndex], nameValue);
        if (isPresent) {
            parcBuffer_Release(&firstSegmentHash);
            parcBuffer_Release(&nameValue);
            return count - 1;
        }
        parcBuffer_Release(&nameValue);
    }

    parcBuffer_Release(&firstSegmentHash);
    return -1;
}
