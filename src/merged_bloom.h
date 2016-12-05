//
// Created by caw on 11/29/16.
//

#ifndef FIB_PERF_MERGED_BLOOM_H
#define FIB_PERF_MERGED_BLOOM_H

#include "name.h"

#include <parc/algol/parc_BitVector.h>

struct merged_bloom_filter;
typedef struct merged_bloom_filter MergedBloomFilter;

MergedBloomFilter *mergedBloomFilter_Create(int N, int m, int k); // N and m determine the matrix dimensions
void mergedBloomFilter_Destroy(MergedBloomFilter **bfP);

void mergedBloomFilter_Add(MergedBloomFilter *filter, Name *name, PARCBitVector *vector);
bool mergedBloomFilter_Lookup(MergedBloomFilter *filter, Name *name);

#endif //FIB_PERF_MERGED_BLOOM_H
