#ifndef bloom_h_
#define bloom_h_

#include <parc/algol/parc_Buffer.h>

struct bloom_filter;
typedef struct bloom_filter BF;

BF *bf_Create(int m, int k);
void bf_Delete(BF **bfP);
void bf_Add(BF *filter, PARCBuffer *value);
bool bf_Test(BF *filter, PARCBuffer *value);

#endif // bloom_h_
