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
} _LinkedBucketEntry;

struct bucket {
    int capacity;
    int numEntries;
    _LinkedBucketEntry **entries;
    struct bucket *overflow;
};
typedef struct bucket _LinkedBucket;

struct map {
    int numBuckets;
    bool rehash;

    int keySize;
    PARCBuffer *key;

    MapMode mode;
    MapOverflowStrategy strategy;

    _LinkedBucket **buckets;
};

_LinkedBucketEntry *
_bucketEntry_Create(PARCBuffer *key, void *item)
{
    _LinkedBucketEntry *entry = (_LinkedBucketEntry *) malloc(sizeof(_LinkedBucketEntry));
    if (entry != NULL) {
        entry->key = parcBuffer_Acquire(key);
        entry->item = item;
    }
    return entry;
}

_LinkedBucket *
_bucket_Create(int capacity)
{
    _LinkedBucket *bucket = (_LinkedBucket *) malloc(sizeof(_LinkedBucket));
    if (bucket != NULL) {
        bucket->numEntries = 0;
        bucket->capacity = capacity;
        if (capacity < 0) {
            bucket->entries = NULL;
        } else {
            bucket->entries = (_LinkedBucketEntry **) malloc(sizeof(_LinkedBucketEntry *) * capacity);
        }
    }
    return bucket;
}

Map *
map_Create(int initialBucketCount, int bucketCapacity, bool rehash, MapMode mode, MapOverflowStrategy strategy)
{
    Map *map = (Map *) malloc(sizeof(Map));

    if (map != NULL) {
        map->keySize = DefaultKeySize;
        map->key = random_Bytes(parcBuffer_Allocate(SiphashKeySize));

        map->mode = mode;
        map->strategy = strategy;
        map->rehash = rehash;

        map->numBuckets = initialBucketCount;
        map->buckets = (_LinkedBucket **) malloc(sizeof(_LinkedBucket *) * initialBucketCount);
        for (int i = 0; i < initialBucketCount; i++) {
            map->buckets[i] = _bucket_Create(bucketCapacity);
        }
    }

    return map;
}

static void
_bucket_AppendItem(_LinkedBucket *bucket, PARCBuffer *key, void *item)
{
    if (bucket->capacity == -1) {
        if (bucket->entries == NULL) {
            bucket->entries = malloc(sizeof(_LinkedBucketEntry *) * (bucket->numEntries + 1));
        } else {
            bucket->entries = realloc(bucket->entries, sizeof(_LinkedBucketEntry *) * (bucket->numEntries + 1));
        }
    }
    bucket->entries[bucket->numEntries++] = _bucketEntry_Create(key, item);
}

static bool
_bucket_InsertItem(_LinkedBucket *bucket, PARCBuffer *key, void *item)
{
    if (bucket->numEntries < bucket->capacity || bucket->capacity == -1) {
        _bucket_AppendItem(bucket, key, item);
        return true;
    }
    return false;
}

static void *
_bucket_GetItem(_LinkedBucket *bucket, PARCBuffer *key)
{
    for (int i = 0; i < bucket->numEntries; i++) {
        _LinkedBucketEntry *target = bucket->entries[i];
        if (parcBuffer_Equals(key, target->key)) {
            return target->item;
        }
    }

    if (bucket->overflow != NULL) {
        return _bucket_GetItem(bucket->overflow, key);
    } else {
        return NULL;
    }
}

static void
_map_InsertToOverflowBucket(Map *map, _LinkedBucket *bucket, PARCBuffer *key, void *item)
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
    _LinkedBucket *bucket = map->buckets[bucketNumber];
    bool wasAdded = _bucket_InsertItem(bucket, key, item);
    if (!wasAdded) {
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

static void *
_map_GetFromBucket(Map *map, int bucketNumber, PARCBuffer *key)
{
    _LinkedBucket *bucket = map->buckets[bucketNumber];
    return _bucket_GetItem(bucket, key);
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
    PARCBuffer *keyHash = key;
    if (map->rehash) {
        keyHash = _map_ComputeBucketKeyHash(map, key);
    }

    int bucketNumber = _map_ComputeBucketNumberFromHash(map, keyHash);
    return _map_GetFromBucket(map, bucketNumber, key);
}
