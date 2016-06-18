#ifndef map_h_
#define map_h_

#include <parc/algol/parc_Buffer.h>

struct map;
typedef struct map Map;

typedef enum {
    MapOverflowStrategy_OverflowBucket,
    MapOverflowStrategy_ExpandAndReHash,
} MapOverflowStrategy;

extern const int MapDefaultCapacity;

Map *map_CreateWithLinkedBuckets(MapOverflowStrategy strategy, bool rehash);
Map *map_CreateWithCompactArray(MapOverflowStrategy strategy, bool rehash);
void map_Destroy(Map **map);

void map_Insert(Map *map, PARCBuffer *key, void *item);
void *map_Get(Map *map, PARCBuffer *key);

#endif // map_h_
