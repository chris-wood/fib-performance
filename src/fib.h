#ifndef fib_h_
#define fib_h_

#include <parc/algol/parc_BitVector.h>
#include <ccnx/common/ccnx_Name.h>

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

typedef struct {
    // The algorithm-specific structure
    void *context;

    // Perform LPM to retrieve the name
    PARCBitVector *(*LPM)(struct fib *map, const CCNxName *ccnxName);

    // Insert a new name into the FIB
    bool (*Insert)(struct fib *map, const CCNxName *ccnxName, PARCBitVector *vector);
} FIB;

FIB *fib_Create(FIBAlgorithm algorithm, FIBMode mode);
PARCBitVector *fib_Lookup(FIB *map, const CCNxName *ccnxName);
bool fib_Insert(FIB *map, const CCNxName *ccnxName, PARCBitVector *vector);

#endif // fib_h_
