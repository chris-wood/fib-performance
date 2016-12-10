#include "fib.h"

struct fib {
    void *instance;
    FIBInterface *interface;
};

FIB *
fib_Create(void *instance, FIBInterface *interface)
{
    FIB *map = (FIB *) malloc(sizeof(FIB));
    if (map != NULL) {
        map->instance = instance;
        map->interface = interface;
    }
    return map;
}

void
fib_Destroy(FIB **fibP)
{
    FIB *fib = *fibP;
    if (fib != NULL) {
        fib->interface->Destroy(&fib->instance);
    }
    free(fib);
    *fibP = NULL;
}

Bitmap *
fib_LPM(FIB *map, const Name *ccnxName)
{
    return map->interface->LPM(map->instance, ccnxName);
}

bool
fib_Insert(FIB *map, const Name *ccnxName, Bitmap *vector)
{
    return map->interface->Insert(map->instance, ccnxName, vector);
}
