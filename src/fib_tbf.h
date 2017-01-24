#ifndef fib_tbf_h
#define fib_tbf_h

#include "fib.h"

struct fib_tbf;
typedef struct fib_tbf FIBTBF;

FIBTBF *fibTBF_Create(int T, int m, int k);
void fibTBF_Destroy(FIBTBF **fibP);

extern FIBInterface *TBFAsFIB;

bool fibTBF_Insert(FIBTBF *fib, const Name *name, Bitmap *vector);
Bitmap *fibTBF_LPM(FIBTBF *fib, const Name *name);

#endif
