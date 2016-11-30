#ifndef fib_cisco_h
#define fib_cisco_h

#include "fib.h"

struct fib_cisco;
typedef struct fib_cisco FIBCisco;

extern FIBInterface *CiscoFIBAsFIB;

FIBCisco *fibCisco_Create(int M);
void fibCisco_Destroy(FIBCisco **fibP);

bool fibCisco_Insert(FIBCisco *fib, const Name *name, PARCBitVector *vector);
PARCBitVector *fibCisco_LPM(FIBCisco *fib, const Name *name);

#endif
