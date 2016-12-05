#ifndef map_h_
#define map_h_

#include <parc/algol/parc_Buffer.h>

struct map;
typedef struct map Map;

extern const int MapDefaultCapacity;

Map *map_Create(void (*delete)(void **instance));
void map_Destroy(Map **map);

void map_Insert(Map *map, PARCBuffer *key, void *item);
void *map_Get(Map *map, PARCBuffer *key);

void map_InsertHashed(Map *map, PARCBuffer *key, void *item);
void *map_GetHashed(Map *map, PARCBuffer *key);

#endif // map_h_
