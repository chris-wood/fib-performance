#ifndef fib_naive_h
#define fib_naive_h

#include <parc/algol/parc_BitVector.h>
#include <ccnx/common/ccnx_Name.h>

struct fib_naive;
typedef struct fib_naive FIBNaive;

FIBNaive *fibNative_Create();

bool fibNaive_Insert(FIBNaive *fib, const CCNxName *name, PARCBitVector *vector);
PARCBitVector *fibNaive_LPM(FIBNaive *fib, const CCNxName *name);

#endif
