#ifndef map_h_
#define map_h_

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

Map *map_Create(int initialBucketCount, int bucketCapacity, bool rehash, MapMode mode, MapOverflowStrategy strategy);
void map_Destroy(Map **map);

void map_Insert(Map *map, PARCBuffer *key, void *item);
void *map_Get(Map *map, PARCBuffer *key);

#endif // map_h_
