#ifndef fibmap_h_
#define fibmap_h_

struct fibmap;
typedef struct fibmap FIBMap;

typedef enum {
    FIBMapMode_Hash,
    FIBMapMode_NoHash
} FIBMapMode;

FIBMap *fibmap_Create(FIBMapMode mode);

PARCBitVector *fibmap_Lookup(FIBMap *map, const CCNxName *ccnxName);

bool fibmap_Insert(FIBMap *map, const CCNxName *ccnxName, PARCBitVector *vector);

#endif // fibmap_h_
