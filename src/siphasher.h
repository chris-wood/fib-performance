#ifdef __cplusplus
extern "C" {
#endif

#ifndef FIB_PERF_SIPHASHER_H
#define FIB_PERF_SIPHASHER_H

#include "hasher.h"

#define SIPHASH_KEY_LENGTH 16
#define SIPHASH_HASH_LENGTH 8

struct siphasher;
typedef struct siphasher SipHasher;

extern HasherInterface *SipHashAsHasher;

SipHasher *siphasher_Create(PARCBuffer *key);

SipHasher *siphasher_CreateWithKeys(int numKeys, PARCBuffer *keys[numKeys]);

void siphasher_Destroy(SipHasher **hasherP);

PARCBuffer *siphasher_Hash(SipHasher *hasher, PARCBuffer *input);

PARCBuffer *siphasher_HashArray(SipHasher *hasher, size_t length, uint8_t input[length]);

Bitmap *siphasher_HashToVector(SipHasher *hasher, PARCBuffer *input, int range);

Bitmap *siphasher_HashArrayToVector(SipHasher *hasher, size_t length, uint8_t input[length], int range);

#endif //FIB_PERF_SIPHASHER_H

#ifdef __cplusplus
}
#endif
