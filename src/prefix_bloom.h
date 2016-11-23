//
// Created by Christopher Wood on 11/22/16.
//

#ifndef FIB_PERF_PREFIX_BLOOM_H
#define FIB_PERF_PREFIX_BLOOM_H

#include <parc/algol/parc_Buffer.h>

// A Prefix Bloom Filter, as described by Varvello et al. in
//   http://conferences.sigcomm.org/sigcomm/2012/paper/icn/p73.pdf
// is a data structure that takes a list of strings as input
// and returns the index j in the list such that i=1,..,j elements
// are "in" the filter.

struct prefix_bloom_filter;
typedef struct prefix_bloom_filter PrefixBloomFilter;

PrefixBloomFilter *prefixBloomFilter_Create(int b, int m, int k);
void prefixBloomFilter_Destroy(PrefixBloomFilter **bfP);

void prefixBloomFilter_Add(PrefixBloomFilter *filter, PARCBuffer *value);
int prefixBloomFilter_LPM(PrefixBloomFilter *filter, PARCBuffer *value);

#endif //FIB_PERF_PREFIX_BLOOM_H
