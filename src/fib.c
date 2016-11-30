#include "fib.h"
#include "fib_cisco.h"
#include "fib_naive.h"

#include <stdint.h>

#include <parc/algol/parc_SafeMemory.h>

struct fib {
    void *instance;
    FIBInterface *interface;
};

FIB *
fib_Create(void *instance, FIBInterface *interface)
{
    FIB *map = (FIB *) malloc(sizeof(FIB));
    if (map != NULL) {
        map->instance = instance;
        map->interface = interface;
    }

    // if (map != NULL) {
    //     switch (algorithm) {
    //         case FIBAlgorithm_Naive:
    //             map->context = fibNative_Create();
    //             map->LPM = (PARCBitVector *(*)(struct fib *, const CCNxName *)) fibNaive_LPM;
    //             map->Insert = (bool (*)(struct fib *, const CCNxName *, PARCBitVector *)) fibNaive_Insert;
    //             break;
    //
    //         case FIBAlgorithm_Cisco:
    //             map->context = fibCisco_Create();
    //             map->LPM = (PARCBitVector *(*)(struct fib *, const CCNxName *)) fibCisco_LPM;
    //             map->Insert = (bool (*)(struct fib *, const CCNxName *, PARCBitVector *)) fibCisco_Insert;
    //             break;
    //
    //         case FIBAlgorithm_Caesar:
    //         case FIBAlgorithm_Song:
    //         default:
    //             break;
    //     }
    // }
    return map;
}

void
fib_Destroy(FIB **fibP)
{
    FIB *fib = *fibP;
    if (fib != NULL) {
        fib->interface->Destroy(&fib->instance);
    }
    free(fib);
    *fibP = NULL;
}

PARCBitVector *
fib_LPM(FIB *map, const Name *ccnxName)
{
    return map->interface->LPM(map->instance, ccnxName);
}

bool
fib_Insert(FIB *map, const Name *ccnxName, PARCBitVector *vector)
{
    return map->interface->Insert(map->instance, ccnxName, vector);
}
