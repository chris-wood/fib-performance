#ifndef bloom_h_
#define bloom_h_

#include <parc/algol/parc_Buffer.h>

struct bloom_filter;
typedef struct bloom_filter BF;

BF *bloom_Create(int m, int k);

void bloom_Delete(BF **bfP);
void bloom_Add(BF *filter, PARCBuffer *value);
bool bloom_Test(BF *filter, PARCBuffer *value);

#endif // bloom_h_
