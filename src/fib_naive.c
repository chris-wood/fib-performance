#include "fib.h"
#include "map.h"

#include <stdio.h>

#include <parc/algol/parc_SafeMemory.h>

typedef struct {
    int numMaps;
    Map **maps;
} NaiveFIB;

static PARCBitVector *
_fibNaive_Lookup(NaiveFIB *fib, const CCNxName *name)
{
    size_t numSegments = ccnxName_GetSegmentCount(name);
    size_t count = numSegments > fib->numMaps ? fib->numMaps : numSegments;

    PARCBitVector *vector = NULL;
    for (size_t i = 0; i < count - 1; i++) {
        CCNxName *copy = ccnxName_Trim(ccnxName_Copy(name), numSegments - (i + 1));
        char *nameString = ccnxName_ToString(copy);
        PARCBuffer *buffer = parcBuffer_AllocateCString(nameString);
        parcMemory_Deallocate(&nameString);

        PARCBitVector *result = map_Get(fib->maps[i], buffer);
        if (result == NULL) {
            return vector;
        } else {
            vector = result;
        }
    }

    char *nameString = ccnxName_ToString(name);
    PARCBuffer *buffer = parcBuffer_AllocateCString(nameString);
    parcMemory_Deallocate(&nameString);

    PARCBitVector *result = map_Get(fib->maps[numSegments - 1], buffer);
    result = result == NULL ? vector : result;
    return result;
}

static Map *
_fibNative_CreateMap()
{
    return map_Create(MapDefaultCapacity, 100, true, MapMode_LinkedBuckets, MapOverflowStrategy_OverflowBucket);
}

static bool
_fibNaive_Insert(NaiveFIB *fib, const CCNxName *name, PARCBitVector *vector)
{
    size_t numSegments = ccnxName_GetSegmentCount(name);
    if (fib->numMaps < numSegments) {
        if (fib->maps == NULL) {
            fib->maps = (Map **) malloc(numSegments * (sizeof(Map *)));
        } else {
            fib->maps = (Map **) realloc(fib->maps, numSegments * (sizeof(Map *)));
        }

        for (size_t i = fib->numMaps; i < numSegments; i++) {
            fib->maps[i] = _fibNative_CreateMap();
        }
        fib->numMaps = numSegments;
    }

    for (size_t i = 0; i < numSegments - 1; i++) {
        CCNxName *copy = ccnxName_Trim(ccnxName_Copy(name), numSegments - (i + 1));
        char *nameString = ccnxName_ToString(copy);
        PARCBuffer *buffer = parcBuffer_AllocateCString(nameString);
        parcMemory_Deallocate(&nameString);

        map_Insert(fib->maps[i], buffer, (void *) vector);
        parcBuffer_Release(&buffer);
    }

    char *nameString = ccnxName_ToString(name);
    PARCBuffer *buffer = parcBuffer_AllocateCString(nameString);
    parcMemory_Deallocate(&nameString);

    map_Insert(fib->maps[numSegments - 1], buffer, (void *) vector);
    parcBuffer_Release(&buffer);

    return true;
}

void *
fibNative_Create(FIB *map)
{
    map->Lookup = _fibNaive_Lookup;
    map->Insert = _fibNaive_Insert;

    NaiveFIB *native = (NaiveFIB *) malloc(sizeof(NaiveFIB));
    native->numMaps = 0;
    native->maps = NULL;

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

            case FIBAlgorithm_Cisco:
                _fibCisco_Create(map);
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
fib_Lookup(FIB *map, const CCNxName *ccnxName)
{
    return map->Lookup(map->context, ccnxName);
}

bool
fib_Insert(FIB *map, const CCNxName *ccnxName, PARCBitVector *vector)
{
    return map->Insert(map->context, ccnxName, vector);
}
