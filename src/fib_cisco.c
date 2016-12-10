#include <stdio.h>

#include "fib_cisco.h"
#include "map.h"

typedef struct {
    bool isVirtual;
    int maxDepth;
    PARCBuffer *buffer;
    Bitmap *vector;
} _FIBCiscoEntry;

struct fib_cisco {
    int M;
    int numMaps;
    Map **maps;
};

static void
_fibCisco_DeleteEntry(_FIBCiscoEntry **entryP)
{
    _FIBCiscoEntry *entry = *entryP;

    if (entry->buffer != NULL) {
        parcBuffer_Release(&entry->buffer);
    }
    free(entry);
    *entryP = NULL;
}

static _FIBCiscoEntry *
_fibCisco_CreateVirtualEntry(int depth)
{
    _FIBCiscoEntry *entry = (_FIBCiscoEntry *) malloc(sizeof(_FIBCiscoEntry));
    if (entry != NULL) {
        entry->isVirtual = true;
        entry->maxDepth = depth;
        entry->vector = NULL;
        entry->buffer = NULL;
    }
    return entry;
}

static _FIBCiscoEntry *
_fibCisco_CreateEntry(Bitmap *vector, PARCBuffer *buffer, int depth)
{
    _FIBCiscoEntry *entry = (_FIBCiscoEntry *) malloc(sizeof(_FIBCiscoEntry));
    if (entry != NULL) {
        entry->isVirtual = false;
        entry->maxDepth = depth;
        entry->vector = vector;
        entry->buffer = parcBuffer_Acquire(buffer);
    }
    return entry;
}

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

static PARCBuffer *
_computeNameBuffer(FIBCisco *fib, const Name *prefix, int numSegments)
{
    return name_GetWireFormat(prefix, numSegments);
}

static _FIBCiscoEntry *
_lookupNamePrefix(FIBCisco *fib, const Name *prefix, int numSegments)
{
    PARCBuffer *buffer = _computeNameBuffer(fib, prefix, numSegments);
    _FIBCiscoEntry *entry = NULL;
    if (name_IsHashed(prefix)) {
        entry = map_GetHashed(fib->maps[numSegments - 1], buffer);
    } else {
        entry = map_Get(fib->maps[numSegments - 1], buffer);
    }
    parcBuffer_Release(&buffer);
    return entry;
}

static void
_insertNamePrefix(FIBCisco *fib, const Name *prefix, int numSegments, _FIBCiscoEntry *entry)
{
    PARCBuffer *buffer = _computeNameBuffer(fib, prefix, numSegments);
    if (name_IsHashed(prefix)) {
        map_InsertHashed(fib->maps[numSegments - 1], buffer, entry);
    } else {
        map_Insert(fib->maps[numSegments - 1], buffer, entry);
    }
    parcBuffer_Release(&buffer);
}

Bitmap *
fibCisco_LPM(FIBCisco *fib, const Name *name)
{
    int numSegments = name_GetSegmentCount(name);

    // prefixCount = min(numSegments, M)
    int prefixCount = MIN(MIN(fib->M, numSegments), fib->numMaps);
    int startPrefix = MIN(numSegments, fib->numMaps);

    _FIBCiscoEntry *firstEntryMatch = NULL;
    for (int i = prefixCount; i > 0; i--) {
        _FIBCiscoEntry *entry = _lookupNamePrefix(fib, name, i); 

        if (entry != NULL) {
            if (entry->maxDepth > fib->M && numSegments > fib->M) {
                startPrefix = MIN(numSegments, entry->maxDepth); 
                firstEntryMatch = entry;
                break;
            } else if (!entry->isVirtual) {
                return entry->vector;
            } 
        }
    }

    if (firstEntryMatch == NULL) {
        return NULL;
    }

    _FIBCiscoEntry *entry = NULL;
    for (int i = startPrefix; i >= 1; i--) {
        if (startPrefix == fib->M) {
            entry = firstEntryMatch;
            if (!entry->isVirtual) {
                return entry->vector;
            }
        }

        _FIBCiscoEntry *targetEntry = _lookupNamePrefix(fib, name, i); 
        if (targetEntry != NULL && !targetEntry->isVirtual) {
            return targetEntry->vector;
        } 
    }

    return NULL;
}

static Map *
_fibCisco_CreateMap()
{
    return map_Create((void (*)(void **)) _fibCisco_DeleteEntry);
}

static void
_fibCisco_ExpandMapsToSize(FIBCisco *fib, int number)
{
    if (fib->numMaps <= number) {
        fib->maps = (Map **) realloc(fib->maps, (number + 1) * (sizeof(Map *)));
        for (size_t i = fib->numMaps; i < number; i++) {
            fib->maps[i] = _fibCisco_CreateMap();
        }
        fib->numMaps = number;
    }
}

bool
fibCisco_Insert(FIBCisco *fib, const Name *name, Bitmap *vector)
{
    size_t numSegments = name_GetSegmentCount(name);
    _fibCisco_ExpandMapsToSize(fib, numSegments);

    // Check to see if we need to create a virtual FIB entry.
    // This occurs when numSegments > M
    // If a FIB entry already exists for M segments, real or virtual, update the MD
    size_t maximumDepth = numSegments;
    if (numSegments > fib->M) {
        _FIBCiscoEntry *entry = _lookupNamePrefix(fib, name, fib->M); 

        if (entry == NULL) {
            entry = _fibCisco_CreateVirtualEntry(maximumDepth);
            _insertNamePrefix(fib, name, fib->M, entry);
        } else if (entry->maxDepth < maximumDepth) {
            entry->maxDepth = MAX(entry->maxDepth, maximumDepth);
        }
    }

    // Update the MD for all segments smaller
    for (size_t i = 1; i < numSegments; i++) {
        _FIBCiscoEntry *entry = _lookupNamePrefix(fib, name, i);
        if (entry != NULL) {
            entry->maxDepth = MAX(entry->maxDepth, maximumDepth);
        }
    }

    PARCBuffer *buffer = _computeNameBuffer(fib, name, numSegments);
    _FIBCiscoEntry *entry = _fibCisco_CreateEntry(vector, buffer, maximumDepth);
    parcBuffer_Release(&buffer);

    // If there is an existing entry, make sure it's *NOT* virtual and update its MD if necessary
    _FIBCiscoEntry *existingEntry = _lookupNamePrefix(fib, name, numSegments);
    if (existingEntry != NULL) {
        existingEntry->isVirtual = false;
        existingEntry->maxDepth = MAX(numSegments, existingEntry->maxDepth);
        if (existingEntry->vector == NULL) {
            existingEntry->vector = vector;
        } else {
            bitmap_SetVector(existingEntry->vector, vector);
        }
    } else {
        _insertNamePrefix(fib, name, numSegments, entry);
    }

    return false;
}

void
fibCisco_Destroy(FIBCisco **fibP)
{
    FIBCisco *fib = *fibP;

    for (int i = 0; i < fib->numMaps; i++) {
        map_Destroy(&fib->maps[i]);
    }
    free(fib->maps);

    free(fib);
    *fibP = NULL;
}

FIBCisco *
fibCisco_Create(int M)
{
    FIBCisco *native = (FIBCisco *) malloc(sizeof(FIBCisco));
    if (native != NULL) {
        native->M = M;
        native->maps = (Map **) malloc(sizeof(Map *));
        native->numMaps = 1;
        native->maps[0] = _fibCisco_CreateMap();
    }

    return native;
}

FIBInterface *CiscoFIBAsFIB = &(FIBInterface) {
        .LPM = (Bitmap *(*)(void *instance, const Name *ccnxName)) fibCisco_LPM,
        .Insert = (bool (*)(void *instance, const Name *ccnxName, Bitmap *vector)) fibCisco_Insert,
        .Destroy = (void (*)(void **instance)) fibCisco_Destroy,
};
