#include <stdlib.h>

#include "bloom.h"

struct bloom_filter {
    int m;
    int k;
};

BF *
bf_Create(int m, int k)
{
    BF *bf = (BF *) malloc(sizeof(BF));
    if (bf != NULL) {
        bf->m = m;
        bf->k = k;
    }
    return bf;
}

void 
bf_Delete(BF **bfP)
{
    // TODO
}

void
bf_Add(BF *filter, PARCBuffer *value)
{

}

bool 
bf_Test(BF *filter, PARCBuffer *value)
{
    return false;
}
