#ifndef fib_h_
#define fib_h_

#include <stdint.h>
#include <stddef.h>

#include <parc/algol/parc_BitVector.h>

#include "name.h"

typedef enum {
    FIBMode_Hash,
    FIBMode_NoHash
} FIBMode;

typedef enum {
    FIBAlgorithm_Naive,
    FIBAlgorithm_Cisco,
    FIBAlgorithm_Caesar,
    FIBAlgorithm_Song
} FIBAlgorithm;

struct fib;
typedef struct fib FIB;

typedef struct {
    // Perform LPM to retrieve the name
    PARCBitVector *(*LPM)(void *instance, const Name *ccnxName);

    // Insert a new name into the FIB
    bool (*Insert)(void *instance, const Name *ccnxName, PARCBitVector *vector);

    void (*Destroy)(void **instance);
} FIBInterface;

FIB *fib_Create(void *instance, FIBInterface *interface);
void fib_Destroy(FIB **fibP);

PARCBitVector *fib_LPM(FIB *map, const Name *ccnxName);
bool fib_Insert(FIB *map, const Name *ccnxName, PARCBitVector *vector);

#endif // fib_h_
