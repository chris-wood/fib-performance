#include "fib.h"

typedef struct {
    // PARCHashMap *hashMap; // TODO: replace with optimal hash map
    int x;
} NaiveFIB;

FIB *
fib_Create(FIBAlgorithm algorithm, FIBMode mode)
{
    FIB *map = (FIB *) malloc(sizeof(FIB));
    if (map != NULL) {
        map->algorithm = algorithm;
        map->mode = mode;
        // TODO: setup the function pointers
    }
    return NULL;
}

PARCBitVector *
fibNaive_Lookup(FIB *map, const CCNxName *ccnxName)
{
    return NULL;
}

bool
fibNaive_Insert(FIB *map, const CCNxName *ccnxName, PARCBitVector *vector)
{
    return false;
}
