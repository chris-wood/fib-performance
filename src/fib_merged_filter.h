//
// Created by caw on 11/29/16.
//

#ifndef FIB_PERF_MERGED_BLOOM_H
#define FIB_PERF_MERGED_BLOOM_H

#include "name.h"
#include "bitmap.h"

struct fib_merged_filter;
typedef struct fib_merged_filter FIBMergedFilter;

FIBMergedFilter *fibMergedFilter_Create(int N, int m, int k); // N and m determine the matrix dimensions
void fibMergedFilter_Destroy(FIBMergedFilter **bfP);

void fibMergedFilter_Add(FIBMergedFilter *filter, Name *name, Bitmap *vector);
Bitmap *fibMergedFilter_Lookup(FIBMergedFilter *filter, Name *name);

#endif //FIB_PERF_MERGED_BLOOM_H
