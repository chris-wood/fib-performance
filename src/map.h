#ifndef fib_h_
#define fib_h_

#include <parc/algol/parc_Buffer.h>

struct map;
typedef struct map Map;

typedef enum {
    MapMode_LinkedBuckets,
    MapMode_CompactBuckets,
} MapMode;

typedef enum {
    MapOverflowStrategy_OverflowBucket,
    MapOverflowStrategy_ExpandAndReHash,
} MapOverflowStrategy;

extern const int MapDefaultCapacity;

Map *map_Create(int initialBucketCount, int bucketCapacity);

void map_Insert(Map *map, PARCBuffer *key, void *item);
void *map_Get(Map *map, PARCBuffer *key);

#endif // fib_h_
