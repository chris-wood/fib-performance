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

struct fib;
typedef struct fib FIB;

typedef struct {
    // Perform LPM to retrieve the name
    PARCBitVector *(*LPM)(void *instance, const CCNxName *ccnxName);

    // Insert a new name into the FIB
    bool (*Insert)(void *instance, const CCNxName *ccnxName, PARCBitVector *vector);
} FIBInterface;

FIB *fib_Create(void *instance, FIBInterface *interface);
PARCBitVector *fib_LPM(FIB *map, const CCNxName *ccnxName);
bool fib_Insert(FIB *map, const CCNxName *ccnxName, PARCBitVector *vector);

#endif // fib_h_
