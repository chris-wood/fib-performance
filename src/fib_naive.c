#include "fib_naive.h"
#include "map.h"

#include <parc/algol/parc_SafeMemory.h>

struct fib_naive {
    int numMaps;
    Map **maps;
};

static PARCBitVector *
_fibNaive_LookupName(FIBNaive *fib, const CCNxName *name)
{
    int numSegments = ccnxName_GetSegmentCount(name);
    char *nameString = ccnxName_ToString(name);
    PARCBuffer *buffer = parcBuffer_AllocateCString(nameString);
    parcMemory_Deallocate(&nameString);

    PARCBitVector *result = map_Get(fib->maps[numSegments - 1], buffer);
    return result;
}

PARCBitVector *
fibNaive_LPM(FIBNaive *fib, const CCNxName *name)
{
    int numSegments = ccnxName_GetSegmentCount(name);
    int count = numSegments > fib->numMaps ? fib->numMaps : numSegments;

    PARCBitVector *vector = NULL;
    for (int i = 0; i < count; i++) {
        CCNxName *copy = ccnxName_Trim(ccnxName_Copy(name), numSegments - (i + 1));
        PARCBitVector *result = _fibNaive_LookupName(fib, copy);
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
    return map_CreateWithLinkedBuckets(MapOverflowStrategy_OverflowBucket, true);
}

bool
fibNaive_Insert(FIBNaive *fib, const CCNxName *name, PARCBitVector *vector)
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

    for (size_t i = 0; i < numSegments; i++) {
        CCNxName *copy = ccnxName_Trim(ccnxName_Copy(name), numSegments - (i + 1));
        char *nameString = ccnxName_ToString(copy);
        PARCBuffer *buffer = parcBuffer_AllocateCString(nameString);
        parcMemory_Deallocate(&nameString);

        map_Insert(fib->maps[i], buffer, (void *) vector);
        parcBuffer_Release(&buffer);
    }

    return true;
}

FIBNaive *
fibNative_Create()
{
    // map->LPM = (PARCBitVector *(*)(struct fib *, const CCNxName *)) _fibNaive_LPM;
    // map->Insert = (bool (*)(struct fib *, const CCNxName *, PARCBitVector *)) _fibNaive_Insert;

    FIBNaive *native = (FIBNaive *) malloc(sizeof(FIBNaive));
    native->numMaps = 0;
    native->maps = NULL;
    return native;
}
