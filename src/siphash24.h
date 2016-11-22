#ifndef siphash24_h_
#define siphash24_h_

#define SIPHASH_KEY_LENGTH 16
#define SIPHASH_HASH_LENGTH 8

int siphash(uint8_t *out, const uint8_t *in, uint64_t inlen, const uint8_t *k);

#endif // siphash24_h_
