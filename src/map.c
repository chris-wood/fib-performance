#include "map.h"
#include "siphash24.h"
#include "random.h"
#include "siphasher.h"

// Some defaults
const int MapDefaultCapacity = 85246;
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
    void (*valueDelete)(void **instance);
    struct bucket *overflow;
};
typedef struct bucket _LinkedBucket;

typedef struct {
    int numBuckets;
    _LinkedBucket **buckets;
    void (*valueDelete)(void **instance);
} _BucketMap;

struct map {
    SipHasher *hasher;
    void (*valueDelete)(void **instance);

    void *instance;
    void (*destroy)(void **);
    void *(*insert)(void *, PARCBuffer *, void *);
    void *(*get)(void *, PARCBuffer *);
};

void
_linkedBucketEntry_Destroy(_LinkedBucketEntry **entryPtr, void (*delete)(void **instance))
{
    _LinkedBucketEntry *entry = *entryPtr;
    parcBuffer_Release(&entry->key);
    if (delete != NULL) {
        delete(&entry->item);
    }
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
        _linkedBucketEntry_Destroy(&entry, bucket->valueDelete);
    }
    free(bucket->entries);

    if (bucket->overflow != NULL) {
        _linkedBucket_Destroy(&bucket->overflow);
    }

    free(bucket);
    *bucketPtr = NULL;
}


_LinkedBucket *
_linkedBucket_Create(int capacity, void (*delete)(void **instance))
{
    _LinkedBucket *bucket = (_LinkedBucket *) malloc(sizeof(_LinkedBucket));
    if (bucket != NULL) {
        bucket->numEntries = 0;
        bucket->capacity = capacity;
        bucket->overflow = NULL;
        bucket->valueDelete = delete;
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
_bucketMap_Create(void (*delete)(void **instance))
{
    _BucketMap *map = (_BucketMap *) malloc(sizeof(_BucketMap));
    map->valueDelete = delete;
    map->numBuckets = MapDefaultCapacity;
    map->buckets = (_LinkedBucket **) malloc(sizeof(_LinkedBucket *) * MapDefaultCapacity);
    for (int i = 0; i < MapDefaultCapacity; i++) {
        map->buckets[i] = _linkedBucket_Create(LinkedBucketDefaultCapacity, delete);
    }
    return map;
}

int
_bucketMap_ComputeBucketNumberFromHash(_BucketMap *map, PARCBuffer *key)
{
    int bucketNumber = 0;
    int length = parcBuffer_Remaining(key);
    uint8_t *overlay = parcBuffer_Overlay(key, 0);
    for (int i = 0; i < length; i++) {
        bucketNumber += overlay[i];
    }

    bucketNumber %= map->numBuckets;

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
        bucket->overflow = _linkedBucket_Create(-1, map->valueDelete);
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
        _bucketMap_InsertToOverflowBucket(map, bucket, key, item);
    }
}

void
map_Destroy(Map **mapPtr)
{
    Map *map = *mapPtr;
    siphasher_Destroy(&map->hasher);
    map->destroy(&map->instance);
    free(map);
    *mapPtr = NULL;
}

Map *
map_Create(void (*delete)(void **instance))
{
    Map *map = (Map *) malloc(sizeof(Map));

    if (map != NULL) {
        PARCBuffer *key = random_Bytes(parcBuffer_Allocate(SiphashKeySize));
        map->hasher = siphasher_Create(key);
        parcBuffer_Release(&key);

        map->valueDelete = delete;

        map->instance = (void *) _bucketMap_Create(delete);
        map->destroy = (void (*)(void **)) _bucketMap_Destroy;
        map->insert = (void *(*)(void *, PARCBuffer *, void *)) _bucketMap_InsertToBucket;
        map->get = (void *(*)(void *, PARCBuffer *)) _bucketMap_Get;
    }

    return map;
}

PARCBuffer *
_map_ComputeBucketKeyHash(Map *map, PARCBuffer *key)
{
    return siphasher_Hash(map->hasher, key);
}

void
map_Insert(Map *map, PARCBuffer *key, void *item)
{
    PARCBuffer *keyHash = _map_ComputeBucketKeyHash(map, key);
    map->insert(map->instance, keyHash, item);
    parcBuffer_Release(&keyHash);
}

void *
map_Get(Map *map, PARCBuffer *key)
{
    PARCBuffer *keyHash = _map_ComputeBucketKeyHash(map, key);

    void *result = map->get(map->instance, keyHash);
    parcBuffer_Release(&keyHash);

    return result;
}

void
map_InsertHashed(Map *map, PARCBuffer *key, void *item)
{
    map->insert(map->instance, key, item);
}

void *
map_GetHashed(Map *map, PARCBuffer *key)
{
    void *result = map->get(map->instance, key);
    return result;
}