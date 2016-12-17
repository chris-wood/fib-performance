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

static uint64_t
_djb(size_t length, uint8_t *buffer) {
    uint64_t hash = 5381;
    int c;

    for (size_t i = 0; i < length; i++) {
        hash = ((hash << 5) + hash) + (buffer[i]); /* hash * 33 + c */
    }
    return hash;
}

static int
_checkSum(size_t length, uint8_t *buffer)
{
    int sum = 0;
    for (size_t i = 0; i < length; i++) {
        sum += buffer[i];
    }
    return sum;
}

void
prefixBloomFilter_Add(PrefixBloomFilter *filter, const Name *name)
{
    PARCBuffer *firstSegmentHash = NULL;
    uint64_t blockIndex = 0;
    if (name_IsHashed(name)) {
        firstSegmentHash = name_GetWireFormat(name, 1);
        blockIndex = _checkSum(parcBuffer_Remaining(firstSegmentHash), parcBuffer_Overlay(firstSegmentHash, 0)) % filter->b;
    } else {
        firstSegmentHash = name_GetWireFormat(name, 1);
        blockIndex = _djb(parcBuffer_Remaining(firstSegmentHash), parcBuffer_Overlay(firstSegmentHash, 0)) % filter->b;
    }
    parcBuffer_Release(&firstSegmentHash);

    if (!name_IsHashed(name)) {
        return bloom_AddName(filter->filterBlocks[blockIndex], name);
    } else {
        // 1. hash first segment to identify the block
        PARCBuffer *firstSegmentHash = name_GetWireFormat(name, 1);
        uint64_t blockIndex = _checkSum(parcBuffer_Remaining(firstSegmentHash), parcBuffer_Overlay(firstSegmentHash, 0)) % filter->b;
        parcBuffer_Release(&firstSegmentHash);

        // 2. add the k hashes to the filter
        PARCBuffer *nameValue = name_GetWireFormat(name, name_GetSegmentCount(name));
        bloom_AddHashed(filter->filterBlocks[blockIndex], nameValue);
        parcBuffer_Release(&nameValue);
    }
}

int
prefixBloomFilter_LPM(PrefixBloomFilter *filter, const Name *name)
{
    PARCBuffer *firstSegmentHash = NULL;
    uint64_t blockIndex = 0;
    if (name_IsHashed(name)) {
        firstSegmentHash = name_GetWireFormat(name, 1);
        blockIndex = _checkSum(parcBuffer_Remaining(firstSegmentHash), parcBuffer_Overlay(firstSegmentHash, 0)) % filter->b;
    } else {
        firstSegmentHash = name_GetWireFormat(name, 1);
        blockIndex = _djb(parcBuffer_Remaining(firstSegmentHash), parcBuffer_Overlay(firstSegmentHash, 0)) % filter->b;
    }
    parcBuffer_Release(&firstSegmentHash);

    if (!name_IsHashed(name)) {
        return bloom_TestName(filter->filterBlocks[blockIndex], name);
    } else {
        for (int count = name_GetSegmentCount(name); count > 0; count--) {
            bool isPresent = false;
            PARCBuffer *nameValue = name_GetWireFormat(name, count);
            isPresent = bloom_TestHashed(filter->filterBlocks[blockIndex], nameValue);
            parcBuffer_Release(&nameValue);

            if (isPresent) {
                return count;
            }
        }

        return -1;
    }
}
