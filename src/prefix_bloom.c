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

        filter->filterBlocks = parcMemory_Allocate(sizeof(PrefixBloomFilter *));
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

    // XXX: clean up

    *bfP = NULL;
}

void
prefixBloomFilter_Add(PrefixBloomFilter *filter, PARCBuffer *value)
{
    // XXX
}

int
prefixBloomFilter_LPM(PrefixBloomFilter *filter, PARCBuffer *value)
{
    // XXX
    return -1;
}
