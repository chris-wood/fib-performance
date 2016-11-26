#include "fib_caesar.h.h"
#include "map.h"

#include <parc/algol/parc_SafeMemory.h>

struct fib_caesar {
    int numMaps;
};

FIBCaesar *
fibCaesar_Create()
{
    FIBCaesar *fib = (FIBCaesar *) malloc(sizeof(FIBCaesar));
    if (fib != NULL) {
        fib->numMaps = 1;
    }
    return fib;
}

FIBInterface *CaesarFIBAsFIB = &(FIBInterface) {
        .LPM = (PARCBitVector *(*)(void *instance, const Name *ccnxName)) fibNaive_Insert,
        .Insert = (bool (*)(void *instance, const Name *ccnxName, PARCBitVector *vector)) fibNaive_LPM,
};

