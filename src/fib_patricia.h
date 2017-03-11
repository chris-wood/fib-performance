#ifdef __cplusplus
extern "C" {
#endif

#ifndef fib_patricia_h_
#define fib_patricia_h_

#include "bitmap.h"
#include "name.h"
#include "fib.h"

struct fib_patricia;
typedef struct fib_patricia FIBPatricia;

extern FIBInterface *PatriciaFIBAsFIB;

FIBPatricia *fibPatricia_Create();

bool fibPatricia_Insert(FIBPatricia *fib, const Name *name, Bitmap *vector);

Bitmap *fibPatricia_LPM(FIBPatricia *fib, const Name *name);

#endif

#ifdef __cplusplus
}
#endif
