#include "fib.h"
#include "fib_cisco.h"
#include "fib_naive.h"

#include <parc/algol/parc_SafeMemory.h>

FIB *
fib_Create(FIBAlgorithm algorithm, FIBMode mode)
{
    FIB *map = (FIB *) malloc(sizeof(FIB));
    if (map != NULL) {
        map->algorithm = algorithm;
        map->mode = mode;

        switch (algorithm) {
            case FIBAlgorithm_Naive:
                map->context = fibNative_Create();
                map->LPM = (PARCBitVector *(*)(struct fib *, const CCNxName *)) fibNaive_LPM;
                map->Insert = (bool (*)(struct fib *, const CCNxName *, PARCBitVector *)) fibNaive_Insert;
                break;

            case FIBAlgorithm_Cisco:
                map->context = fibCisco_Create();
                map->LPM = (PARCBitVector *(*)(struct fib *, const CCNxName *)) fibCisco_LPM;
                map->Insert = (bool (*)(struct fib *, const CCNxName *, PARCBitVector *)) fibCisco_Insert;
                break;

            case FIBAlgorithm_Caesar:
            case FIBAlgorithm_Song:
            default:
                break;
        }
    }
    return map;
}

PARCBitVector *
fib_LPM(FIB *map, const CCNxName *ccnxName)
{
    return map->LPM(map->context, ccnxName);
}

bool
fib_Insert(FIB *map, const CCNxName *ccnxName, PARCBitVector *vector)
{
    bool absent = fib_LPM(map, ccnxName) == NULL;
    if (absent) {
        return map->Insert(map->context, ccnxName, vector);
    }
    return absent;
}
