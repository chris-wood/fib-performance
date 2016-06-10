#include "fib.h"
#include "map.h"

#include <stdio.h>

typedef struct {
    int numMaps;
    Map **maps;
} NaiveFIB;

typedef struct {
    Map *map;
} CiscoFIB;

static PARCBitVector *
_fibNaive_Lookup(NaiveFIB *fib, const CCNxName *name)
{
    size_t numSegments = ccnxName_GetSegmentCount(name);
    size_t count = numSegments > fib->numMaps ? fib->numMaps : numSegments;
    PARCBitVector *vector = NULL;

    for (size_t i = 0; i < count; i++) {
        CCNxName *copy = ccnxName_Trim(ccnxName_Copy(name), numSegments - (i + 1));
        char *nameString = ccnxName_ToString(copy);
        PARCBuffer *buffer = parcBuffer_AllocateCString(nameString);

        PARCBitVector *result = map_Get(fib->maps[i], buffer);
        if (result == NULL) {
            return vector;
        } else {
            vector = result;
        }
    }

    return vector;
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
        fib->maps = realloc(fib->maps, numSegments * (sizeof(Map *)));
        for (size_t i = fib->numMaps; i < numSegments; i++) {
            fib->maps[i] = _fibNative_CreateMap();
        }
    }

    char *nameString = ccnxName_ToString(name);
    PARCBuffer *buffer = parcBuffer_Flip(parcBuffer_AllocateCString(nameString));

    // TODO: map_Insert should return a boolean
    map_Insert(fib->maps[numSegments - 1], buffer, (void *) vector);
    parcBuffer_Release(&buffer);

    return true;
}

static void
_fibNative_Create(FIB *map)
{
    map->Lookup = _fibNaive_Lookup;
    map->Insert = _fibNaive_Insert;

    NaiveFIB *native = (NaiveFIB *) malloc(sizeof(NaiveFIB));
    native->numMaps = 0;
    native->maps = (Map **) malloc(sizeof(Map *));

    map->context = (void *) native;
}

static PARCBitVector *
_fibCisco_Lookup(CiscoFIB *fib, const CCNxName *ccnxName)
{
    return NULL;
}

static bool
_fibCisco_Insert(CiscoFIB *fib, const CCNxName *name, PARCBitVector *vector)
{
    return NULL;
}

static void
_fibCisco_Create(FIB *map)
{
    map->Lookup = _fibCisco_Lookup;
    map->Insert = _fibCisco_Insert;

    // TODO
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
