#include "fib_patricia.h"

#include "patricia.h"

struct fib_patricia {
    Patricia *trie;
};

Bitmap *
fibPatricia_LPM(FIBPatricia *fib, const Name *name)
{
    PARCBuffer *trieKey = name_GetWireFormat(name, name_GetSegmentCount(name));
    Bitmap *vector = (Bitmap *) patricia_Get(fib->trie, trieKey);
    parcBuffer_Release(&trieKey);
    return vector;
}

bool
fibPatricia_Insert(FIBPatricia *fib, const Name *name, Bitmap *vector)
{
    PARCBuffer *trieKey = name_GetWireFormat(name, name_GetSegmentCount(name));
    patricia_Insert(fib->trie, trieKey, vector);
    parcBuffer_Release(&trieKey);
    return true;
}

void
fibPatricia_Destroy(FIBPatricia **fibP)
{
    FIBPatricia *fib = *fibP;
    patricia_Destroy(&fib->trie);
    free(fib);
    *fibP = NULL;
}

FIBPatricia *
fibPatricia_Create()
{
    FIBPatricia *fib = (FIBPatricia *) malloc(sizeof(FIBPatricia));
    if (fib != NULL) {
        fib->trie = patricia_Create(NULL);
    }

    return fib;
}

FIBInterface *PatriciaFIBAsFIB = &(FIBInterface) {
    .LPM = (Bitmap *(*)(void *instance, const Name *ccnxName)) fibPatricia_LPM,
    .Insert = (bool (*)(void *instance, const Name *ccnxName, Bitmap *vector)) fibPatricia_Insert,
    .Destroy = (void (*)(void **instance)) fibPatricia_Destroy,
};
