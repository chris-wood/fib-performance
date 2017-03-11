#ifdef __cplusplus
extern "C" {
#endif

#ifndef FIB_PERF_MERGED_BLOOM_H
#define FIB_PERF_MERGED_BLOOM_H

#include "name.h"
#include "bitmap.h"
#include "fib.h"

struct fib_merged_filter;
typedef struct fib_merged_filter FIBMergedFilter;

extern FIBInterface *MergedFilterFIBAsFIB;

FIBMergedFilter *fibMergedFilter_Create(int N, int m, int k); // N and m determine the matrix dimensions
void fibMergedFilter_Destroy(FIBMergedFilter **bfP);

bool fibMergedFilter_Insert(FIBMergedFilter *filter, Name *name, Bitmap *vector);
Bitmap *fibMergedFilter_LPM(FIBMergedFilter *filter, Name *name);

#endif //FIB_PERF_MERGED_BLOOM_H

#ifdef __cplusplus
}
#endif
