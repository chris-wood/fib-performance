#ifndef fib_caesar_h
#define fib_caesar_h

#include "fib.h"

struct fib_caesar;
typedef struct fib_caesar FIBCaesar;

FIBCaesar *fibCaesar_Create(int b, int m, int k);
void fibCaesar_Destroy(FIBCaesar **fibP);

extern FIBInterface *CaesarFIBAsFIB;

bool fibCaesar_Insert(FIBCaesar *fib, const Name *name, PARCBitVector *vector);
PARCBitVector *fibCaesar_LPM(FIBCaesar *fib, const Name *name);

#endif
