#ifndef fib_naive_h
#define fib_naive_h

#include "fib.h"

struct fib_naive;
typedef struct fib_naive FIBNaive;

FIBNaive *fibNative_Create();
void fibNaive_Destroy(FIBNaive **fibP);

extern FIBInterface *NativeFIBAsFIB;

bool fibNaive_Insert(FIBNaive *fib, const Name *name, Bitmap *vector);
Bitmap *fibNaive_LPM(FIBNaive *fib, const Name *name);

#endif
