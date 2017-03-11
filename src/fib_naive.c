#include "fib_naive.h"
#include "map.h"

struct fib_naive {
    int numMaps;
    Map **maps;
};

static Bitmap *
_fibNaive_LookupName(FIBNaive *fib, const Name *name, int count)
{
    PARCBuffer *buffer = name_GetWireFormat(name, count);
    Bitmap *result = NULL;

    if (name_IsHashed(name)) {
        result = map_GetHashed(fib->maps[count - 1], buffer);
    } else {
        result = map_Get(fib->maps[count - 1], buffer);
    }

    parcBuffer_Release(&buffer);
    return result;
}

Bitmap *
fibNaive_LPM(FIBNaive *fib, const Name *name)
{
    int numSegments = name_GetSegmentCount(name);
    int count = numSegments > fib->numMaps ? fib->numMaps : numSegments;

    Bitmap *vector = NULL;
    for (int i = count; i > 0; i--) {
        Bitmap *result = _fibNaive_LookupName(fib, name, i);
        if (result == NULL && vector != NULL) {
            return vector;
        } else { // vector == NULL
            vector = result;
        }
    }

    return vector;
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
fibNaive_Insert(FIBNaive *fib, const Name *name, Bitmap *vector)
{
    size_t numSegments = name_GetSegmentCount(name);
    if (numSegments < fib->numMaps) {
        Bitmap *lookup = _fibNaive_LookupName(fib, name, numSegments);
        if (lookup != NULL) {
            bitmap_SetVector(lookup, vector);
            return true;
        }
    }

    _fibNative_ExpandMapsToSize(fib, numSegments);

    PARCBuffer *buffer = name_GetWireFormat(name, numSegments);
    if (name_IsHashed(name)) {
        map_InsertHashed(fib->maps[numSegments - 1], buffer, (void *) vector);
    } else {
        map_Insert(fib->maps[numSegments - 1], buffer, (void *) vector);
    }
    parcBuffer_Release(&buffer);

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
        .LPM = (Bitmap *(*)(void *instance, const Name *ccnxName)) fibNaive_LPM,
        .Insert = (bool (*)(void *instance, const Name *ccnxName, Bitmap *vector)) fibNaive_Insert,
        .Destroy = (void (*)(void **instance)) fibNaive_Destroy,
};

