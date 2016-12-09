//
// Created by Christopher Wood on 11/22/16.
//

#include <stdlib.h>

#include <parc/algol/parc_Memory.h>

#include "prefix_bloom.h"

#include "bloom.h"
#include "siphasher.h"

struct prefix_bloom_filter {
    int k;
    int m;
    int b;

    PARCBuffer **keys;
    BloomFilter **filterBlocks;
    SipHasher *hasher;
};

PrefixBloomFilter *
prefixBloomFilter_Create(int b, int m, int k)
{
    PrefixBloomFilter *filter = parcMemory_Allocate(sizeof(PrefixBloomFilter));
    if (filter != NULL) {
        filter->b = b;
        filter->m = m;
        filter->k = k;

        filter->filterBlocks = parcMemory_Allocate(b * sizeof(PrefixBloomFilter *));
        for (int i = 0; i < b; i++) {
            filter->filterBlocks[i] = bloom_Create(m, k);
        }

        filter->keys = (PARCBuffer **) malloc(sizeof(PARCBuffer **) * k);
        for (int i = 0; i < k; i++) {
            filter->keys[i] = parcBuffer_Allocate(SIPHASH_KEY_LENGTH);
            parcBuffer_PutUint32(filter->keys[i], k);
            parcBuffer_Flip(filter->keys[i]);
        }
        filter->hasher = siphasher_CreateWithKeys(k, filter->keys);
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
    for (int i = 0; i < filter->k; i++) {
        parcBuffer_Release(&filter->keys[i]);
    }

    free(filter->keys);
    parcMemory_Deallocate(&filter->filterBlocks);
    siphasher_Destroy(&filter->hasher);

    parcMemory_Deallocate(bfP);
    *bfP = NULL;
}

void
prefixBloomFilter_Add(PrefixBloomFilter *filter, const Name *name)
{
    // 1. hash first segment to identify the block
    PARCBuffer *firstSegmentHash = NULL;
    if (name_IsHashed(name)) {
        firstSegmentHash = name_GetWireFormat(name, 1);
    } else {
        firstSegmentHash = siphasher_HashArray(filter->hasher, name_GetSegmentLength(name, 0), name_GetSegmentOffset(name, 0));
    }
    uint64_t blockIndex = parcBuffer_GetUint64(firstSegmentHash) % filter->b;
    parcBuffer_Release(&firstSegmentHash);

    // 2. add the k hashes to the filter
    PARCBuffer *nameValue = name_GetWireFormat(name, name_GetSegmentCount(name));
    if (name_IsHashed(name)) {
        bloom_AddHashed(filter->filterBlocks[blockIndex], nameValue);
    } else {
        bloom_Add(filter->filterBlocks[blockIndex], nameValue);
    }

    parcBuffer_Release(&nameValue);
}

int
prefixBloomFilter_LPM(PrefixBloomFilter *filter, const Name *name)
{
    // XXX:
    // if (!name_IsHashed(name)):
    //     return bloom_TestName(..) <-- this does the LPM internally
    // else:
    //     do the LPM code here

    // 1. hash first segment to identify the block
    PARCBuffer *firstSegmentHash = NULL;
    if (name_IsHashed(name)) {
        firstSegmentHash = name_GetWireFormat(name, 1);
    } else {
        firstSegmentHash = siphasher_HashArray(filter->hasher, name_GetSegmentLength(name, 0), name_GetSegmentOffset(name, 0));
    }
    uint64_t blockIndex = parcBuffer_GetUint64(firstSegmentHash) % filter->b;
    parcBuffer_Release(&firstSegmentHash);

    // 2. do the LPM lookup, starting with the longest possible name
    for (int count = name_GetSegmentCount(name); count > 0; count--) {
        PARCBuffer *nameValue = name_GetWireFormat(name, count);
        bool isPresent = false;
        if (name_IsHashed(name)) {
            isPresent = bloom_TestHashed(filter->filterBlocks[blockIndex], nameValue);
        } else {
            isPresent = bloom_Test(filter->filterBlocks[blockIndex], nameValue);
        }

        if (isPresent) {
            parcBuffer_Release(&nameValue);
            return count;
        }
        parcBuffer_Release(&nameValue);
    }

    return -1;
}
