#include "map.h"
#include "siphash24.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

const int MapDefaultCapacity = 42;

typedef struct {
    PARCBuffer *key;
    void *item;
} _BucketEntry;

typedef struct {
    int capacity;
    int numEntries;
    _BucketEntry **entries;
} _Bucket;

struct map {
    int numBuckets;
    bool rehash;

    MapMode mode;
    MapOverflowStrategy strategy;

    _Bucket **buckets;
};

static uint8_t *
_randomBytes(int n)
{
    FILE *f = fopen("/dev/urandom", "r");
    uint8_t *bytes = NULL;
    if (f != NULL) {
        bytes = (uint8_t *) malloc(n);
        int numRead = fread(bytes, sizeof(uint8_t), n, f);
        if (numRead != n) {
            fprintf(stderr, "Failed to read %d bytes, got %d instead", n, numRead);
            free(bytes);
        }
        fclose(f);
    }
    return bytes;
}

_BucketEntry *
_bucketEntry_Create(PARCBuffer *key, void *item)
{
    _BucketEntry *entry = (_BucketEntry *) malloc(sizeof(_BucketEntry));
    if (entry != NULL) {
        entry->key = parcBuffer_Acquire(key);
        entry->item = item;
    }
    return entry;
}

_Bucket *
_bucket_Create(int capacity)
{
    _Bucket *bucket = (_Bucket *) malloc(sizeof(_Bucket));
    if (bucket != NULL) {
        bucket->numEntries = 0;
        bucket->capacity = capacity;
        bucket->entries = (_BucketEntry **) malloc(sizeof(_BucketEntry *) * capacity);
    }
    return bucket;
}

Map *
map_Create(int initialBucketCount, int bucketCapacity, bool rehash, MapMode mode, MapOverflowStrategy strategy)
{
    Map *map = (Map *) malloc(sizeof(Map));
    if (map != NULL) {
        map->numBuckets = initialBucketCount;

        map->buckets = (_Bucket **) malloc(sizeof(_Bucket *) * initialBucketCount);
        map->mode = mode;
        map->strategy = strategy;
        map->rehash = rehash;

        for (int i = 0; i < initialBucketCount; i++) {
            map->buckets[i] = _bucket_Create(bucketCapacity);
        }
    }
    return map;
}

static void
_map_InsertToBucket(Map *map, int bucketNumber, PARCBuffer *key, void *item)
{
    _Bucket *bucket = map->buckets[bucketNumber];
    if (bucket->numEntries < bucket->capacity) {
        bucket->entries[bucket->numEntries++] = _bucketEntry_Create(key, item);
    } else { // overflow
        // TODO: add another bucket -- with linear or dynamic hashing? hmm...
    }
}

int
_map_ComputeBucketNumberFromHash(Map *map, PARCBuffer *key)
{
    int bucketNumber = 0;
    while (parcBuffer_Remaining(key) > 0) {
        bucketNumber += parcBuffer_GetUint8(key);
    }

    bucketNumber %= map->numBuckets;
    parcBuffer_Flip(key);

    return bucketNumber;
}

PARCBuffer *
_map_ComputeBucketKeyHash(Map *map, PARCBuffer *key)
{
    // TODO: invoke SipHash here (as per Cisco paper)
    return key;
}

void
map_Insert(Map *map, PARCBuffer *key, void *item)
{
    PARCBuffer *keyHash = key;
    if (map->rehash) {
        keyHash = _map_ComputeBucketKeyHash(map, key);
    }

    int bucketNumber = _map_ComputeBucketNumberFromHash(map, keyHash);
    _map_InsertToBucket(map, bucketNumber, key, item);
}

void *
map_Get(Map *map, PARCBuffer *key)
{
    return NULL;
}
