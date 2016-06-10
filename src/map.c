#include "map.h"
#include "siphash24.h"
#include "random.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// Some defaults
const int MapDefaultCapacity = 42;
const int DefaultKeySize = 64 / 8;
const int SiphashKeySize = 128 / 8;

typedef struct {
    PARCBuffer *key;
    void *item;
} _BucketEntry;

struct bucket {
    int capacity;
    int numEntries;
    _BucketEntry **entries;
    struct bucket *overflow;
};
typedef struct bucket _Bucket;

struct map {
    int numBuckets;
    bool rehash;

    int keySize;
    PARCBuffer *key;

    MapMode mode;
    MapOverflowStrategy strategy;

    _Bucket **buckets;
};

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
        map->keySize = DefaultKeySize;
        map->key = random_Bytes(parcBuffer_Allocate(SiphashKeySize));

        map->mode = mode;
        map->strategy = strategy;
        map->rehash = rehash;

        map->buckets = (_Bucket **) malloc(sizeof(_Bucket *) * initialBucketCount);
        for (int i = 0; i < initialBucketCount; i++) {
            map->buckets[i] = _bucket_Create(bucketCapacity);
        }
    }

    return map;
}

static void
_bucket_AppendItem(_Bucket *bucket, PARCBuffer *key, void *item)
{
    if (bucket->capacity == -1) {
        bucket->entries = realloc(bucket->entries, sizeof(_BucketEntry *) * (bucket->numEntries + 1));
    }
    bucket->entries[bucket->numEntries++] = _bucketEntry_Create(key, item);
}

static bool
_bucket_InsertItem(_Bucket *bucket, PARCBuffer *key, void *item)
{
    if (bucket->numEntries < bucket->capacity || bucket->capacity == -1) {
        _bucket_AppendItem(bucket, key, item);
        return true;
    }
    return false;
}

static void
_map_InsertToOverflowBucket(Map *map, _Bucket *bucket, PARCBuffer *key, void *item)
{
    if (bucket->overflow == NULL) {
        bucket->overflow = _bucket_Create(-1);
    }
    _bucket_InsertItem(bucket->overflow, key, item);
}

static void
_map_ExpandAndRehash(Map *map, PARCBuffer *key, void *item)
{
    // TODO
}

static void
_map_InsertToBucket(Map *map, int bucketNumber, PARCBuffer *key, void *item)
{
    _Bucket *bucket = map->buckets[bucketNumber];
    bool full = _bucket_InsertItem(bucket, key, item);
    if (full) {
        switch (map->strategy) {
            case MapOverflowStrategy_OverflowBucket:
                _map_InsertToOverflowBucket(map, bucket, key, item);
                break;
            case MapOverflowStrategy_ExpandAndReHash:
                // _map_ExpandAndRehash(map, key, item);
                break;
        }
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
    PARCBuffer *buffer = parcBuffer_Allocate(map->keySize);
    uint8_t *output = parcBuffer_Overlay(buffer, 0);

    uint8_t *input = parcBuffer_Overlay(key, 0);
    size_t length = parcBuffer_Remaining(key);

    uint8_t *hashKey = parcBuffer_Overlay(map->key, 0);

    int result = siphash(output, input, length, hashKey);

    return buffer;
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
