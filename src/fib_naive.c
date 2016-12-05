#include "fib_naive.h"
#include "map.h"

struct fib_naive {
    int numMaps;
    Map **maps;
};

static PARCBitVector *
_fibNaive_LookupName(FIBNaive *fib, const Name *name, int count)
{
    PARCBuffer *buffer = name_GetWireFormat(name, count);
    PARCBitVector *result = NULL;

    if (name_IsHashed(name)) {
        result = map_GetHashed(fib->maps[count - 1], buffer);
    } else {
        result = map_Get(fib->maps[count - 1], buffer);
    }

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
        PARCBitVector *result = _fibNaive_LookupName(fib, name, i);
        if (result == NULL && vector != NULL) {
            return parcBitVector_Acquire(vector);
        } else { // vector == NULL
            vector = result;
        }
    }

    if (vector != NULL) {
        return parcBitVector_Acquire(vector);
    } else {
        return vector;
    }
}

static Map *
_fibNative_CreateMap()
{
    return map_Create(NULL);
}

static void
_fibNative_ExpandMapsToSize(FIBNaive *fib, int number)
{
    if (fib->numMaps <= number) {
        fib->maps = (Map **) realloc(fib->maps, (number + 1) * (sizeof(Map *)));
        for (size_t i = fib->numMaps; i < number; i++) {
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

        // XXX: caw, insert hashed
        map_Insert(fib->maps[i], buffer, (void *) vector);
        parcBuffer_Release(&buffer);
    }

    return true;
}

void
fibNaive_Destroy(FIBNaive **fibP)
{
    FIBNaive *fib = *fibP;

    for (int i = 0; i < fib->numMaps; i++) {
        map_Destroy(&fib->maps[i]);
    }
    free(fib->maps);

    free(fib);
    *fibP = NULL;
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
        .Destroy = (void (*)(void **instance)) fibNaive_Destroy,
};

