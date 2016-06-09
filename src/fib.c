#include "fib.h"
#include "map.h"

#include <stdio.h>

typedef struct {
    Map *map;
} NaiveFIB;

static PARCBitVector *
_fibNaive_Lookup(NaiveFIB *fib, const CCNxName *ccnxName)
{
    char *nameString = ccnxName_ToString(ccnxName);
    PARCBuffer *buffer = parcBuffer_AllocateCString(nameString);

    PARCBitVector *vector = map_Get(fib->map, buffer);
    return vector;
}

static bool
_fibNaive_Insert(NaiveFIB *fib, const CCNxName *name, PARCBitVector *vector)
{
    char *nameString = ccnxName_ToString(name);
    PARCBuffer *buffer = parcBuffer_Flip(parcBuffer_AllocateCString(nameString));

    // TODO: map_Insert should return a boolean
    map_Insert(fib->map, buffer, (void *) vector);
    parcBuffer_Release(&buffer);

    return true;
}

static void
_fibNative_Create(FIB *map)
{
    map->Lookup = _fibNaive_Lookup;
    map->Insert = _fibNaive_Insert;

    NaiveFIB *native = (NaiveFIB *) malloc(sizeof(NaiveFIB));
    native->map = map_Create(MapDefaultCapacity, 100, true, MapMode_LinkedBuckets, MapOverflowStrategy_OverflowBucket);

    map->context = (void *) native;
}

FIB *
fib_Create(FIBAlgorithm algorithm, FIBMode mode)
{
    FIB *map = (FIB *) malloc(sizeof(FIB));
    if (map != NULL) {
        map->algorithm = algorithm;
        map->mode = mode;

        switch (algorithm) {
            case FIBAlgorithm_Naive:
                _fibNative_Create(map);
                break;

            // TODO: finish the code for these ones
            case FIBAlgorithm_Cisco:
            case FIBAlgorithm_Caesar:
            case FIBAlgorithm_Song:
            default:
                break;
        }
    }
    return map;
}

PARCBitVector *
fib_Lookup(FIB *map, const CCNxName *ccnxName)
{
    return map->Lookup(map->context, ccnxName);
}

bool
fib_Insert(FIB *map, const CCNxName *ccnxName, PARCBitVector *vector)
{
    return map->Insert(map->context, ccnxName, vector);
}
