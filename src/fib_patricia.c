#include "fib_patricia.h"

#include "patricia.h"

#include <parc/algol/parc_SafeMemory.h>

struct fib_patricia {
    Patricia *trie;
};

PARCBitVector *
fibPatricia_LPM(FIBPatricia *fib, const Name *name)
{
    return NULL; 
}

bool
fibPatricia_Insert(FIBPatricia *fib, const Name *name, Bitmap *vector)
{
    return false;
}

FIBPatricia *
fibPatricia_Create()
{
    FIBPatricia *fib = (FIBPatricia *) malloc(sizeof(FIBPatricia));
    if (fib != NULL) {
        fib = patricia_Create();
    }

    return fib;
}

FIBInterface *PatriciaFIBAsFIB = &(FIBInterface) {
    .LPM = (Bitmap *(*)(void *instance, const Name *ccnxName)) fibPatricia_LPM,
    .Insert = (bool (*)(void *instance, const Name *ccnxName, Bitmap *vector)) fibPatricia_Insert,
};
