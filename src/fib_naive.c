#include "fib_naive.h"
#include "map.h"

#include <parc/algol/parc_SafeMemory.h>

struct fib_naive {
    int numMaps;
    Map **maps;
};

static PARCBitVector *
_fibNaive_LookupName(FIBNaive *fib, const Name *name, int count)
{
    PARCBuffer *buffer = name_GetWireFormat(name, count);
    PARCBitVector *result = map_Get(fib->maps[count - 1], buffer);
    parcBuffer_Release(&buffer);
    return result;
}

PARCBitVector *
fibNaive_LPM(FIBNaive *fib, const Name *name)
{
    int numSegments = name_GetSegmentCount(name);
    int count = numSegments > fib->numMaps ? fib->numMaps : numSegments;

    PARCBitVector *vector = NULL;
    for (int i = count; i > 0; i--) {
        PARCBitVector *result = _fibNaive_LookupName(fib, name, count);
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

static void
_fibNative_ExpandMapsToSize(FIBNaive *fib, int number)
{
    if (fib->numMaps < number) {
        fib->maps = (Map **) realloc(fib->maps, (number + 1) * (sizeof(Map *)));
        for (size_t i = fib->numMaps; i <= number; i++) {
            fib->maps[i] = _fibNative_CreateMap();
        }
        fib->numMaps = number;
    }
}

bool
fibNaive_Insert(FIBNaive *fib, const Name *name, PARCBitVector *vector)
{
    PARCBitVector *lookup = fibNaive_LPM(fib, name);
    if (lookup != NULL) {
        parcBitVector_SetVector(lookup, vector);
        return true;
    }

    size_t numSegments = name_GetSegmentCount(name);
    _fibNative_ExpandMapsToSize(fib, numSegments);

    for (size_t i = 0; i < numSegments; i++) {
        PARCBuffer *buffer = name_GetWireFormat(name, i + 1);
        map_Insert(fib->maps[i], buffer, (void *) vector);
        parcBuffer_Release(&buffer);
    }

    return true;
}

FIBNaive *
fibNative_Create()
{
    FIBNaive *native = (FIBNaive *) malloc(sizeof(FIBNaive));
    if (native != NULL) {
        native->numMaps = 1;
        native->maps = (Map **) malloc(sizeof(Map *));
        native->maps[0] = _fibNative_CreateMap();
    }
    return native;
}

FIBInterface *NativeFIBAsFIB = &(FIBInterface) {
    .LPM = (PARCBitVector *(*)(void *instance, const Name *ccnxName)) fibNaive_LPM,
    .Insert = (bool (*)(void *instance, const Name *ccnxName, PARCBitVector *vector)) fibNaive_Insert,
};

