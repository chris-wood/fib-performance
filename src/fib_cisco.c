#include "fib_cisco.h"
#include "map.h"

#include <parc/algol/parc_SafeMemory.h>

typedef struct {
    bool isVirtual;
    int maxDepth;
    PARCBitVector *vector;
} _FIBCiscoEntry;

struct fib_cisco {
    int M;
    int numMaps;
    Map **maps;
};

static _FIBCiscoEntry *
_fibCisco_CreateVirtualEntry(int depth)
{
    _FIBCiscoEntry *entry = (_FIBCiscoEntry *) malloc(sizeof(_FIBCiscoEntry));
    if (entry != NULL) {
        entry->isVirtual = true;
        entry->maxDepth = depth;
        entry->vector = NULL;
    }
    return entry;
}

static _FIBCiscoEntry *
_fibCisco_CreateEntry(PARCBitVector *vector, int depth)
{
    _FIBCiscoEntry *entry = (_FIBCiscoEntry *) malloc(sizeof(_FIBCiscoEntry));
    if (entry != NULL) {
        entry->isVirtual = false;
        entry->maxDepth = depth;
        entry->vector = parcBitVector_Acquire(vector);;
    }
    return entry;
}

PARCBitVector *
fibCisco_LPM(FIBCisco *fib, const CCNxName *name)
{
    int numSegments = ccnxName_GetSegmentCount(name);
    int prefixCount = numSegments < fib->M ? numSegments - 1 : fib->M;
    int startPrefix = numSegments - 1;
    _FIBCiscoEntry *firstEntryMatch = NULL;

    for (int i = prefixCount; i >= 0; i--) {
        CCNxName *copy = ccnxName_Trim(ccnxName_Copy(name), numSegments - (i + 1));
        char *nameString = ccnxName_ToString(copy);
        PARCBuffer *buffer = parcBuffer_AllocateCString(nameString);
        parcMemory_Deallocate(&nameString);

        _FIBCiscoEntry *entry = map_Get(fib->maps[i], buffer);
        if (entry != NULL) {
            if (entry->maxDepth > fib->M && numSegments > fib->M) {
                startPrefix = numSegments <= entry->maxDepth ? startPrefix : entry->maxDepth - 1;
                firstEntryMatch = entry;
                break;
            } else if (!entry->isVirtual) {
                return entry->vector;
            }
        }
        parcBuffer_Release(&buffer);
    }

    for (int i = startPrefix; i >= 0; i--) {
        CCNxName *copy = ccnxName_Trim(ccnxName_Copy(name), numSegments - (i + 1));
        char *nameString = ccnxName_ToString(copy);
        PARCBuffer *buffer = parcBuffer_AllocateCString(nameString);
        parcMemory_Deallocate(&nameString);

        _FIBCiscoEntry *entry = map_Get(fib->maps[i], buffer);
        if (entry != NULL) {
            if (!entry->isVirtual) {
                return entry->vector;
            }
        }
        parcBuffer_Release(&buffer);
    }

    return NULL;
}

static Map *
_fibCisco_CreateMap()
{
    return map_CreateWithLinkedBuckets(MapOverflowStrategy_OverflowBucket, true);
}

static void
_fibCisco_ExpandMapsToSize(FIBCisco *fib, int number)
{
    if (fib->numMaps < number) {
        fib->maps = (Map **) realloc(fib->maps, number * (sizeof(Map *)));
        for (size_t i = fib->numMaps; i < number; i++) {
            fib->maps[i] = _fibCisco_CreateMap();
        }
        fib->numMaps = number;
    }
}

bool
fibCisco_Insert(FIBCisco *fib, const CCNxName *name, PARCBitVector *vector)
{
    size_t numSegments = ccnxName_GetSegmentCount(name);
    _fibCisco_ExpandMapsToSize(fib, numSegments);

    size_t maximumDepth = numSegments;
    if (numSegments < fib->M) {
        CCNxName *copy = ccnxName_Trim(ccnxName_Copy(name), numSegments - (fib->M + 1));
        char *nameString = ccnxName_ToString(copy);
        PARCBuffer *buffer = parcBuffer_AllocateCString(nameString);
        parcMemory_Deallocate(&nameString);

        _FIBCiscoEntry *entry = map_Get(fib->maps[fib->M], buffer);
        if (entry == NULL) {
            entry = _fibCisco_CreateVirtualEntry(maximumDepth);
            map_Insert(fib->maps[fib->M], buffer, (void *) entry);
        } else if (entry->maxDepth < maximumDepth) {
            entry->maxDepth = maximumDepth;
        }
        parcBuffer_Release(&buffer);
    }

    for (size_t i = 0; i < numSegments - 1; i++) {
        CCNxName *copy = ccnxName_Trim(ccnxName_Copy(name), numSegments - (i + 1));
        char *nameString = ccnxName_ToString(copy);
        PARCBuffer *buffer = parcBuffer_AllocateCString(nameString);
        parcMemory_Deallocate(&nameString);

        _FIBCiscoEntry *entry = map_Get(fib->maps[fib->M], buffer);
        if (entry != NULL) {
            entry->maxDepth = maximumDepth;
        }
    }

    char *nameString = ccnxName_ToString(name);
    PARCBuffer *buffer = parcBuffer_AllocateCString(nameString);
    parcMemory_Deallocate(&nameString);

    _FIBCiscoEntry *entry = _fibCisco_CreateEntry(vector, maximumDepth);
    map_Insert(fib->maps[numSegments - 1], buffer, (void *) entry);
    parcBuffer_Release(&buffer);

    return false;
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
    .LPM = (PARCBitVector *(*)(void *instance, const CCNxName *ccnxName)) fibCisco_LPM,
    .Insert = (bool (*)(void *instance, const CCNxName *ccnxName, PARCBitVector *vector)) fibCisco_Insert,
};
