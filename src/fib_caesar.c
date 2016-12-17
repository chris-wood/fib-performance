#include "fib_caesar.h"

#include "map.h"
#include "prefix_bloom.h"

struct fib_caesar {
    int numMaps;
    PrefixBloomFilter *pbf;
    Map **maps;
};

static Map *
_fibCaesar_CreateMap()
{
    return map_Create(NULL);
}

void
fibCaesar_Destroy(FIBCaesar **fibP)
{
    FIBCaesar *fib = *fibP;

    for (int i = 0; i < fib->numMaps; i++) {
        map_Destroy(&fib->maps[i]);
    }
    free(fib->maps);
    prefixBloomFilter_Destroy(&fib->pbf);

    free(fib);
    *fibP = NULL;
}

FIBCaesar *
fibCaesar_Create(int b, int m, int k)
{
    FIBCaesar *fib = (FIBCaesar *) malloc(sizeof(FIBCaesar));
    if (fib != NULL) {
        fib->pbf = prefixBloomFilter_Create(b, m, k);
        fib->numMaps = 1;
        fib->maps = (Map **) malloc(sizeof(Map *));
        fib->maps[0] = _fibCaesar_CreateMap();
    }
    return fib;
}

static void
_fibCaesar_ExpandMapsToSize(FIBCaesar *fib, int number)
{
    if (fib->numMaps <= number) {
        fib->maps = (Map **) realloc(fib->maps, (number + 1) * (sizeof(Map *)));
        for (size_t i = fib->numMaps; i < number; i++) {
            fib->maps[i] = _fibCaesar_CreateMap();
        }
        fib->numMaps = number;
    }
}

Bitmap *
fibCaesar_LPM(FIBCaesar *fib, const Name *name)
{
    int numMatches = prefixBloomFilter_LPM(fib->pbf, name);
    if (numMatches > 0 && numMatches <= fib->numMaps) {
        Map *table = fib->maps[numMatches - 1];

        PARCBuffer *key = name_GetWireFormat(name, numMatches);
        Bitmap *match = NULL;
        if (name_IsHashed(name)) {
            match = map_GetHashed(table, key);
        } else {
            match = map_Get(table, key);
        }

        parcBuffer_Release(&key);
        return match;
    }
    return NULL;
}

bool
fibCaesar_Insert(FIBCaesar *fib, const Name *name, Bitmap *vector)
{
    Bitmap *match = fibCaesar_LPM(fib, name);
    if (match != NULL) {
        bitmap_SetVector(match, vector);
    }

    prefixBloomFilter_Add(fib->pbf, name);

    int numSegments = name_GetSegmentCount(name);
    _fibCaesar_ExpandMapsToSize(fib, numSegments);
    Map *table = fib->maps[numSegments - 1];

    PARCBuffer *key = name_GetWireFormat(name, numSegments);
    if (name_IsHashed(name)) {
        map_InsertHashed(table, key, vector);
    } else {
        map_Insert(table, key, vector);
    }
    parcBuffer_Release(&key);

    return true;
}

FIBInterface *CaesarFIBAsFIB = &(FIBInterface) {
        .LPM = (Bitmap *(*)(void *instance, const Name *ccnxName)) fibCaesar_LPM,
        .Insert = (bool (*)(void *instance, const Name *ccnxName, Bitmap *vector)) fibCaesar_Insert,
        .Destroy = (void (*)(void **instance)) fibCaesar_Destroy,
};

