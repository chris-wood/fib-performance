#ifndef fib_caesar_filter_h
#define fib_caesar_filter_h

#include "fib.h"

struct fib_caesar_filter;
typedef struct fib_caesar_filter FIBCaesarFilter;

FIBCaesarFilter* fibCaesarFilter_Create(int numPorts, int b, int m, int k);
void fibCaesarFilter_Destroy(FIBCaesarFilter **fibP);

extern FIBInterface *CaesarFilterFIBAsFIB;

bool fibCaesarFilter_Insert(FIBCaesarFilter *fib, const Name *name, PARCBitVector *vector);
PARCBitVector *fibCaesarFilter_LPM(FIBCaesarFilter *fib, const Name *name);

#endif
