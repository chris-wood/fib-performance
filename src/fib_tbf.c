#include "fib_tbf.h"

#include "map.h"
#include "patricia.h"
#include "bloom.h"

typedef struct {
    int type;
    Bitmap *vector;

    BloomFilter **filters;
    int numMaps;
} _fibEntry;

typedef enum {
    _FIBEntryType_Bitmap,
    _FIBEntryType_BF,
} _FIBEntryType;

struct fib_tbf {
    int T;
    int m;
    int k;
    Patricia *trie;
    Map *map;
};

Bitmap *
fibTBF_LPM(FIBTBF *fib, const Name *name)
{
    int numSegments = name_GetSegmentCount(name);
    bool isShortName = numSegments < fib->T;

    PARCBitVector *tSegment = name_GetWireFormat(name, MIN(fib->T, numSegments);
    _fibEntry *trieLookup = patricia_Get(fib->trie, tSegment);

    if (trieLookup == NULL) {
        return NULL;
    } else if (isShortName) {
        return trieLookup->vector;
    } else {
        // Lookup prefix with the BFs
        Bitmap **maps = entry->filters;

        // Expand the set of filters, if necessary
        int B = numSegments - fib->T;
        if (B > entry->numMaps) {
            entry->filters = (BloomFilter **) realloc(entry->filters, B);
            for (int i = entry->numMaps; i < B; i++) {
                entry->filters[i] = bloom_Create(fib->m, fib->k);
            }
            entry->numMaps = B;
        }

        // Find the longest prefix
        int i = 0;
        for (i = MIN(entry->numMaps, numSegments); i > fib->T; i--) {
            PARCBuffer *subPrefix = name_GetSubWireFormat(name, T, T + i);
            if (bloom_Test(entry->filters, subPrefix)) {
                parcBuffer_Release(&subPrefix);
                break;
            }
            parcBuffer_Release(&subPrefix);
        }

        PARCBuffer *matchingPrefix = name_GetWireFormat(name, fib->T + i);
        return map_Get(fib->map, matchingPrefix);
    }

    return NULL;
}

#define MIN(a, b) (a < b ? a : b)

bool
fibTBF_Insert(FIBTBF *fib, const Name *name, Bitmap *egressVector)
{
    int numSegments = name_GetSegmentCount(name);
    bool isShortName = numSegments < fib->T;

    PARCBitVector *tSegment = name_GetWireFormat(name, MIN(fib->T, numSegments);
    _fibEntry *trieLookup = patricia_Get(fib->trie, tSegment);

    if (trieLookup == NULL) {

    } else {
        if (isShortName) {

        } else {

        }
    }

//    _fibEntry *entry = malloc(sizeof(_fibEntry));
//    if (numSegments < fib->T) {
//        entry->type = _FIBEntryType_Bitmap;
//        entry->value = (void *) egressVector;
//    } else {
//        entry->type = _FIBEntryType_BF;
//        entry->value = (void *) prefixBloomFilter_Create(fib->b, fib->m, fib->k);
//    }
//
//    if (isShortName) {
//        if (trieLookup == NULL) { // XXX: create new entry
//
//        } else if (isExactMatch) { // merge
//
//        } else { // add new entry
//
//        }
//        // XXX: need to check if we're inserting a new value or merging...
////        patricia_Insert(fib->trie, tSegment, egressVector);
//    } else {
//        if (trieLookup == NULL) { // add new BF
//
//        } else { // add to existing PBF
//
//        }
//    }

    // If N < T, insert item that contains bitmap
    // Else, insert item that contains PBF
    // for both, append the items created to a list so they can be deleted later on
    return true;
}

void
fibTBF_Destroy(FIBTBF **fibP)
{
    FIBTBF *fib = *fibP;

    patricia_Destroy(&fib->trie);

    free(fib);
    *fibP = NULL;
}

FIBTBF *
fibNative_Create(int T, int m, int k)
{
    FIBTBF *fib= (FIBTBF *) malloc(sizeof(FIBTBF));
    if (fib != NULL) {
        fib->T = T;
        fib->m = m;
        fib->k = k;
        fib->trie = patricia_Create();
        fib->map = map_Create(bitmap_Destroy);
    }
    return fib;
}

FIBInterface *NativeFIBAsFIB = &(FIBInterface) {
        .LPM = (Bitmap *(*)(void *instance, const Name *ccnxName)) fibTBF_LPM,
        .Insert = (bool (*)(void *instance, const Name *ccnxName, Bitmap *vector)) fibTBF_Insert,
        .Destroy = (void (*)(void **instance)) fibTBF_Destroy,
};

