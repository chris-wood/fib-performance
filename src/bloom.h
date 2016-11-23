#ifndef bloom_h_
#define bloom_h_

#include <parc/algol/parc_Buffer.h>

struct bloom_filter;
typedef struct bloom_filter BloomFilter;

BloomFilter *bloom_Create(int m, int k);
void bloom_Destroy(BloomFilter **bfP);

void bloom_Add(BloomFilter *filter, PARCBuffer *value);
bool bloom_Test(BloomFilter *filter, PARCBuffer *value);

#endif // bloom_h_
