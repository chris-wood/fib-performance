#include "fib.h"
#include "map.h"

#include <stdio.h>

#include <parc/algol/parc_SafeMemory.h>

typedef struct {
    int numMaps;
    Map **maps;
} NaiveFIB;

typedef struct {
    bool isVirtual;
    int maxDepth;
    PARCBitVector *vector;
} CiscoFIBEntry;

typedef struct {
    int M;
    int numMaps;
    Map **maps;
} CiscoFIB;

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

static void
_fibNative_Create(FIB *map)
{
    map->Lookup = _fibNaive_Lookup;
    map->Insert = _fibNaive_Insert;

    NaiveFIB *native = (NaiveFIB *) malloc(sizeof(NaiveFIB));
    native->numMaps = 0;
    native->maps = NULL;

    map->context = (void *) native;
}

static CiscoFIBEntry *
_fibCisco_CreateVirtualEntry(int depth)
{
    CiscoFIBEntry *entry = (CiscoFIBEntry *) malloc(sizeof(CiscoFIBEntry));
    if (entry != NULL) {
        entry->isVirtual = true;
        entry->maxDepth = depth;
        entry->vector = NULL;
    }
    return entry;
}

static CiscoFIBEntry *
_fibCisco_CreateEntry(PARCBitVector *vector, int depth)
{
    CiscoFIBEntry *entry = (CiscoFIBEntry *) malloc(sizeof(CiscoFIBEntry));
    if (entry != NULL) {
        entry->isVirtual = false;
        entry->maxDepth = depth;
        entry->vector = parcBitVector_Acquire(vector);;
    }
    return entry;
}

static PARCBitVector *
_fibCisco_Lookup(CiscoFIB *fib, const CCNxName *name)
{
    size_t numSegments = ccnxName_GetSegmentCount(name);
    size_t prefixCount = numSegments < fib->M ? numSegments : fib->M;
    size_t startPrefix = 0;
    CiscoFIBEntry *firstEntryMatch = NULL;

    for (size_t i = prefixCount; prefixCount > 0; i--) {
        CCNxName *copy = ccnxName_Trim(ccnxName_Copy(name), numSegments - (i + 1));
        char *nameString = ccnxName_ToString(copy);
        PARCBuffer *buffer = parcBuffer_AllocateCString(nameString);
        parcMemory_Deallocate(&nameString);

        CiscoFIBEntry *entry = map_Get(fib->maps[i], buffer);
        if (entry != NULL) {
            if (entry->maxDepth > fib->M && numSegments > fib->M) {
                startPrefix = numSegments < entry->maxDepth ? numSegments : entry->maxDepth;
                firstEntryMatch = entry;
                break;
            }
        }
        parcBuffer_Release(&buffer);
    }

    for (size_t i = startPrefix; startPrefix > 0; i--) {
        if (startPrefix == fib->M) {
            CiscoFIBEntry *entry = firstEntryMatch;
            if (!entry->isVirtual) {
                return entry;
            }
        }

        CCNxName *copy = ccnxName_Trim(ccnxName_Copy(name), numSegments - (i + 1));
        char *nameString = ccnxName_ToString(copy);
        PARCBuffer *buffer = parcBuffer_AllocateCString(nameString);
        parcMemory_Deallocate(&nameString);

        CiscoFIBEntry *entry = map_Get(fib->maps[i], buffer);
        if (entry != NULL) {
            if (!entry->isVirtual) {
                return entry;
            }
        }
        parcBuffer_Release(&buffer);
    }

    return NULL;
}

static bool
_fibCisco_Insert(CiscoFIB *fib, const CCNxName *name, PARCBitVector *vector)
{
    size_t numSegments = ccnxName_GetSegmentCount(name);

    // TODO: pull this out into a function
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

    size_t maximumDepth = numSegments;
    size_t virtualStartLength = numSegments / fib->M;

    while (virtualStartLength > 0) {
        CCNxName *copy = ccnxName_Trim(ccnxName_Copy(name), numSegments - (virtualStartLength + 1));
        char *nameString = ccnxName_ToString(copy);
        PARCBuffer *buffer = parcBuffer_AllocateCString(nameString);
        parcMemory_Deallocate(&nameString);

        CiscoFIBEntry *entry = _fibCisco_CreateVirtualEntry(numSegments);
        map_Insert(fib->maps[virtualStartLength], buffer, (void *) entry);
        parcBuffer_Release(&buffer);

        virtualStartLength -= fib->M;
    }

    for (size_t i = 0; i < numSegments - 1; i++) {
        CCNxName *copy = ccnxName_Trim(ccnxName_Copy(name), numSegments - (i + 1));
        char *nameString = ccnxName_ToString(copy);
        PARCBuffer *buffer = parcBuffer_AllocateCString(nameString);
        parcMemory_Deallocate(&nameString);

        CiscoFIBEntry *entry = _fibCisco_CreateEntry(vector, numSegments);
        map_Insert(fib->maps[i], buffer, (void *) entry);
        parcBuffer_Release(&buffer);
    }

    return false;
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
