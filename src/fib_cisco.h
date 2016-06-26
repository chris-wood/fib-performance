#ifndef fib_cisco_h
#define fib_cisco_h

#include <parc/algol/parc_BitVector.h>
#include <ccnx/common/ccnx_Name.h>

struct fib_cisco;
typedef struct fib_cisco FIBCisco;

FIBCisco *fibCisco_Create(int M);

bool fibCisco_Insert(FIBCisco *fib, const CCNxName *name, PARCBitVector *vector);
PARCBitVector *fibCisco_LPM(FIBCisco *fib, const CCNxName *name);

#endif
