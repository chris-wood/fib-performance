//
// Created by caw on 11/29/16.
//

#include "merged_bloom.h"
#include "bloom.h"

struct merged_bloom_filter {
    int N;
    int m;
    int k;

    BloomFilter **filters;
};

void
mergedBloomFilter_Destroy(MergedBloomFilter **filterP)
{
    MergedBloomFilter *filter = *filterP;

    for (int i = 0; i < filter->N; i++) {
        bloom_Destroy(&filter->filters[i]);
    }
    free(filter->filters);

    free(filter);
    *filterP = NULL;
}

MergedBloomFilter *
mergedBloomFilter_Create(int N, int m, int k) // N and m determine the matrix dimensions
{
    MergedBloomFilter *filter = (MergedBloomFilter *) malloc(sizeof(MergedBloomFilter));
    if (filter != NULL) {
        filter->N = N;
        filter->m = m;
        filter->k = k;

        filter->filters = (BloomFilter **) malloc(sizeof(BloomFilter *) * N);
        for (int i = 0; i < N; i++) {
            filter->filters[i] = bloom_Create(m, k);
        }
    }
    return filter;
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
