//
// Created by caw on 11/29/16.
//

#include "merged_bloom.h"

struct merged_bloom_filter {
    int N;
    int m;
    int k;
};

MergedBloomFilter *
mergedBloomFilter_Create(int N, int m, int k) // N and m determine the matrix dimensions
{
    MergedBloomFilter *filter = (MergedBloomFilter *) malloc(sizeof(MergedBloomFilter));
    if (filter != NULL) {
        filter->N = N;
        filter->m = m;
        filter->k = k;

        // XXX
    }
    return filter;
}

void
mergedBloomFilter_Destroy(MergedBloomFilter **bfP)
{

}

void
mergedBloomFilter_Add(MergedBloomFilter *filter, Name *name, PARCBitVector *vector)
{

}

bool
mergedBloomFilter_Lookup(MergedBloomFilter *filter, Name *name)
{
    return false;
}
