//
// Created by caw on 11/23/16.
//

#ifndef FIB_PERF_HASHER_H
#define FIB_PERF_HASHER_H

#include <parc/algol/parc_Buffer.h>
#include <parc/algol/parc_BitVector.h>

struct hasher;
typedef struct hasher Hasher;

Hasher *hasher_Create();
void hasher_Destroy(Hasher **hasherP);

PARCBuffer *hasher_Hash(Hasher *hasher, PARCBuffer *input, PARCBuffer *key);
PARCBitVector *hasher_HashToVector(Hasher *hasher, PARCBuffer *input, int range, int numKeys, PARCBuffer **keys);

#endif //FIB_PERF_HASHER_H
