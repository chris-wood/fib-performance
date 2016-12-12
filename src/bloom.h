#ifndef bloom_h_
#define bloom_h_

#include <parc/algol/parc_Buffer.h>

#include "name.h"

struct bloom_filter;
typedef struct bloom_filter BloomFilter;

BloomFilter *bloom_Create(int m, int k);
void bloom_Destroy(BloomFilter **bfP);

void bloom_Add(BloomFilter *filter, PARCBuffer *value);
bool bloom_Test(BloomFilter *filter, PARCBuffer *value);

void bloom_AddRaw(BloomFilter *filter, int length, uint8_t value[length]);
bool bloom_TestRaw(BloomFilter *filter, int length, uint8_t value[length]);

void bloom_AddHashed(BloomFilter *filter, PARCBuffer *value);
bool bloom_TestHashed(BloomFilter *filter, PARCBuffer *value);

void bloom_AddName(BloomFilter *filter, Name *name);
int bloom_TestName(BloomFilter *filter, Name *name);

#endif // bloom_h_
