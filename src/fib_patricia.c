#include "fib_patricia.h"

#include <parc/algol/parc_SafeMemory.h>

PARCBitVector *
fibPatricia_LPM(FIBPatricia *fib, const CCNxName *name)
{
    return NULL; 
}

bool
fibPatricia_Insert(FIBPatricia *fib, const CCNxName *name, PARCBitVector *vector)
{
    return false;
}

FIBPatricia *
fibPatricia_Create()
{
    FIBPatricia *native = (FIBPatricia *) malloc(sizeof(FIBPatricia));
    if (native != NULL) {
        // XXX: 
    }

    return native;
}

FIBInterface *PatriciaFIBAsFIB = &(FIBInterface) {
    .LPM = (PARCBitVector *(*)(void *instance, const CCNxName *ccnxName)) fibPatricia_Insert,
    .Insert = (bool (*)(void *instance, const CCNxName *ccnxName, PARCBitVector *vector)) fibPatricia_LPM,
};
