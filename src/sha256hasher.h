#ifdef __cplusplus
extern "C" {
#endif

#ifndef FIB_PERF_SHA256HASHER_H
#define FIB_PERF_SHA256HASHER_H

#include <parc/algol/parc_Buffer.h>

#include "hasher.h"

struct sha256hasher;
typedef struct sha256hasher SHA256Hasher;

extern HasherInterface *SHA256HashAsHasher;

SHA256Hasher *sha256hasher_Create();

void sha256hasher_Destroy(SHA256Hasher **hasherP);

PARCBuffer *sha256hasher_Hash(SHA256Hasher *hasher, PARCBuffer *input);

PARCBuffer *sha256hasher_HashArray(SHA256Hasher *hasher, size_t length, uint8_t input[length]);

Bitmap *sha256hasher_HashToVector(SHA256Hasher *hasher, PARCBuffer *input, int range);

#endif //FIB_PERF_SHA256HASHER_H

#ifdef __cplusplus
}
#endif
