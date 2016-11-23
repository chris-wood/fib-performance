//
// Created by Christopher Wood on 11/22/16.
//

#ifndef FIB_PERF_PREFIX_BLOOM_H
#define FIB_PERF_PREFIX_BLOOM_H

struct prefix_bloom_filter;
typedef struct prefix_bloom_filter PrefixBloomFilter;

PrefixBloomFilter *prefixBloomFilter_Create(int b, int m, int k);
void prefixBloomFilter_Destroy(PrefixBloomFilter **bfP);

//void bloom_Destroy(BloomFilter **bfP);
//void bloom_Add(BloomFilter *filter, PARCBuffer *value);
//bool bloom_Test(BloomFilter *filter, PARCBuffer *value);


#endif //FIB_PERF_PREFIX_BLOOM_H
