#ifndef fib_patricia_h_
#define fib_patricia_h_

#include <parc/algol/parc_BitVector.h>
#include <ccnx/common/ccnx_Name.h>

#include "fib.h"

struct fib_patricia;
typedef struct fib_patricia FIBPatricia;

extern FIBInterface *PatriciaFIBAsFIB;

FIBPatricia *fibPatricia_Create(int M);

bool fibPatricia_Insert(FIBPatricia *fib, const CCNxName *name, PARCBitVector *vector);
PARCBitVector *fibPatricia_LPM(FIBPatricia *fib, const CCNxName *name);

#endif
