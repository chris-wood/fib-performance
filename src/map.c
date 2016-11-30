#include "map.h"
#include "siphash24.h"
#include "random.h"

// Some defaults
const int MapDefaultCapacity = 42;
const int DefaultKeySize = 64 / 8;
const int SiphashKeySize = 128 / 8;
const int LinkedBucketDefaultCapacity = 100;

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

typedef struct {
    MapOverflowStrategy strategy;
    int numBuckets;
    _LinkedBucket **buckets;
} _BucketMap;

struct map {
    bool rehash;
    int keySize;
    PARCBuffer *key;

    void *instance;
    void (*destroy)(void **);
    void *(*insert)(void *, PARCBuffer *, void *);
    void *(*get)(void *, PARCBuffer *);
};

void
_linkedBucketEntry_Destroy(_LinkedBucketEntry **entryPtr)
{
    _LinkedBucketEntry *entry = *entryPtr;
    parcBuffer_Release(&entry->key);
    free(entry);
    *entryPtr = NULL;
}

_LinkedBucketEntry *
_linkedBucketEntry_Create(PARCBuffer *key, void *item)
{
    _LinkedBucketEntry *entry = (_LinkedBucketEntry *) malloc(sizeof(_LinkedBucketEntry));
    if (entry != NULL) {
        entry->key = parcBuffer_Acquire(key);
        entry->item = item;
    }
    return entry;
}

void
_linkedBucket_Destroy(_LinkedBucket **bucketPtr)
{
    _LinkedBucket *bucket = *bucketPtr;

    for (int i = 0; i < bucket->numEntries; i++) {
        _LinkedBucketEntry *entry = bucket->entries[i];
        _linkedBucketEntry_Destroy(&entry);
    }
    free(bucket->entries);

    if (bucket->overflow != NULL) {
        _linkedBucket_Destroy(&bucket->overflow);
    }

    free(bucket);
    *bucketPtr = NULL;
}


_LinkedBucket *
_linkedBucket_Create(int capacity)
{
    _LinkedBucket *bucket = (_LinkedBucket *) malloc(sizeof(_LinkedBucket));
    if (bucket != NULL) {
        bucket->numEntries = 0;
        bucket->capacity = capacity;
        bucket->overflow = NULL;
        if (capacity < 0) {
            bucket->entries = NULL;
        } else {
            bucket->entries = (_LinkedBucketEntry **) malloc(sizeof(_LinkedBucketEntry *) * capacity);
        }
    }
    return bucket;
}

static void
_bucketMap_Destroy(_BucketMap **mapPtr)
{
    _BucketMap *map = *mapPtr;
    for (int i = 0; i < map->numBuckets; i++) {
        _LinkedBucket *current = map->buckets[i];
        _linkedBucket_Destroy(&current);
    }
    free(map);
    *mapPtr = NULL;
}

static _BucketMap *
_bucketMap_Create(MapOverflowStrategy strategy)
{
    _BucketMap *map = (_BucketMap *) malloc(sizeof(_BucketMap));
    map->strategy = strategy;
    map->numBuckets = MapDefaultCapacity;
    map->buckets = (_LinkedBucket **) malloc(sizeof(_LinkedBucket *) * MapDefaultCapacity);
    for (int i = 0; i < MapDefaultCapacity; i++) {
        map->buckets[i] = _linkedBucket_Create(LinkedBucketDefaultCapacity);
    }
    return map;
}

int
_bucketMap_ComputeBucketNumberFromHash(_BucketMap *map, PARCBuffer *key)
{
    int bucketNumber = 0;
    while (parcBuffer_Remaining(key) > 0) {
        bucketNumber += parcBuffer_GetUint8(key);
    }

    bucketNumber %= map->numBuckets;
    parcBuffer_Flip(key);

    return bucketNumber;
}

static void
_linkedBucket_AppendItem(_LinkedBucket *bucket, PARCBuffer *key, void *item)
{
    if (bucket->capacity == -1) {
        if (bucket->entries == NULL) {
            bucket->entries = malloc(sizeof(_LinkedBucketEntry *));
        } else {
            bucket->entries = realloc(bucket->entries, sizeof(_LinkedBucketEntry *) * (bucket->numEntries + 1));
        }
    }
    bucket->entries[bucket->numEntries++] = _linkedBucketEntry_Create(key, item);
}

static bool
_linkedBucket_InsertItem(_LinkedBucket *bucket, PARCBuffer *key, void *item)
{
    if (bucket->numEntries < bucket->capacity || bucket->capacity == -1) {
        _linkedBucket_AppendItem(bucket, key, item);
        return true;
    }
    return false;
}

static void *
_linkedBucket_GetItem(_LinkedBucket *bucket, PARCBuffer *key)
{
    for (int i = 0; i < bucket->numEntries; i++) {
        _LinkedBucketEntry *target = bucket->entries[i];
        if (parcBuffer_Equals(key, target->key)) {
            return target->item;
        }
    }

    if (bucket->overflow != NULL) {
        return _linkedBucket_GetItem(bucket->overflow, key);
    } else {
        return NULL;
    }
}

static void *
_bucketMap_Get(_BucketMap *map, PARCBuffer *key)
{
    int bucketNumber = _bucketMap_ComputeBucketNumberFromHash(map, key);
    _LinkedBucket *bucket = map->buckets[bucketNumber];
    return _linkedBucket_GetItem(bucket, key);
}

static void
_bucketMap_InsertToOverflowBucket(_BucketMap *map, _LinkedBucket *bucket, PARCBuffer *key, void *item)
{
    if (bucket->overflow == NULL) {
        bucket->overflow = _linkedBucket_Create(-1);
    }
    _linkedBucket_InsertItem(bucket->overflow, key, item);
}

//static void
//_bucketMap_ExpandAndRehash(_BucketMap *map, PARCBuffer *key, void *item)
//{
//    // TODO
//}

static void
_bucketMap_InsertToBucket(_BucketMap *map, PARCBuffer *key, void *item)
{
    int bucketNumber = _bucketMap_ComputeBucketNumberFromHash(map, key);
    _LinkedBucket *bucket = map->buckets[bucketNumber];

    bool wasAdded = _linkedBucket_InsertItem(bucket, key, item);
    if (!wasAdded) {
        switch (map->strategy) {
            case MapOverflowStrategy_OverflowBucket:
                _bucketMap_InsertToOverflowBucket(map, bucket, key, item);
                break;
            case MapOverflowStrategy_ExpandAndReHash:
                assertTrue(false, "MapOverflowStrategy_ExpandAndReHash not implemented yet.");
                // _map_ExpandAndRehash(map, key, item);
                break;
        }
    }
}

void
map_Destroy(Map **mapPtr)
{
    Map *map = *mapPtr;
    parcBuffer_Release(&map->key);
    map->destroy(&map->instance);
    free(map);
    *mapPtr = NULL;
}

Map *
map_CreateWithLinkedBuckets(MapOverflowStrategy strategy, bool rehash)
{
    Map *map = (Map *) malloc(sizeof(Map));

    if (map != NULL) {
        map->keySize = DefaultKeySize;
        map->key = random_Bytes(parcBuffer_Allocate(SiphashKeySize));

        map->rehash = rehash;

        map->instance = (void *) _bucketMap_Create(strategy);
        map->destroy = (void (*)(void **)) _bucketMap_Destroy;
        map->insert = (void *(*)(void *, PARCBuffer *, void *)) _bucketMap_InsertToBucket;
        map->get = (void *(*)(void *, PARCBuffer *)) _bucketMap_Get;
    }

    return map;
}

PARCBuffer *
_map_ComputeBucketKeyHash(Map *map, PARCBuffer *key)
{
    PARCBuffer *buffer = parcBuffer_Allocate(map->keySize);
    assertNotNull(buffer, "Expected a non-NULL buffer to be allocated");
    uint8_t *output = parcBuffer_Overlay(buffer, 0);

    assertNotNull(key, "Expected a non-NULL key to be provided");
    uint8_t *input = parcBuffer_Overlay(key, 0);
    size_t length = parcBuffer_Remaining(key);

    assertNotNull(map->key, "Expected a non-NULL map key to be provided for SIPHASH");
    uint8_t *hashKey = parcBuffer_Overlay(map->key, 0);

    int result = siphash(output, input, length, hashKey);
    assertTrue(result == 0, "SIPHash failed");

    return buffer;
}

void
map_Insert(Map *map, PARCBuffer *key, void *item)
{
    PARCBuffer *keyHash = key;
    if (map->rehash) {
        keyHash = _map_ComputeBucketKeyHash(map, key);
    }

    map->insert(map->instance, keyHash, item);
    if (map->rehash) {
        parcBuffer_Release(&keyHash);
    }
}

void *
map_Get(Map *map, PARCBuffer *key)
{
    PARCBuffer *keyHash = key;
    if (map->rehash) {
        keyHash = _map_ComputeBucketKeyHash(map, key);
    }

    void *result = map->get(map->instance, keyHash);
    if (map->rehash) {
        parcBuffer_Release(&keyHash);
    }
}
