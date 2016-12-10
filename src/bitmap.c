//
// Created by Christopher Wood on 12/9/16.
//

#include "bitmap.h"

#include <stdlib.h>

#include <LongBow/runtime.h>

#define WORDSIZE 32

struct bitmap {
    uint32_t *map;
    int bitSize;
    int byteSize;
};

Bitmap *
bitmap_Create(int size)
{
    // XXX: we should check that this is a multiple of 8
    Bitmap *map = (Bitmap *) malloc(sizeof(Bitmap));
    if (map != NULL) {
        map->bitSize = size;
        map->byteSize = size / 8;
        map->map = (uint32_t *) malloc(sizeof(uint32_t) * (map->byteSize / sizeof(uint32_t)));
    }
    return map;
}

void
bitmap_Destroy(Bitmap **bitmapP)
{
    Bitmap *map = *bitmapP;
    free(map->map);
    free(map);
    *bitmapP = NULL;
}

static int
_bitToBlock(int bit)
{
    int byteOffset = bit / 8;
    int wordOffset = byteOffset / 4;
    return wordOffset;
}

static uint32_t
_bitToBlockMask(int bit)
{
    return (1 << (WORDSIZE - bit - 1));
}

bool
bitmap_Get(Bitmap *bitmap, int bit)
{
    if (bit < 0 || bit > bitmap->bitSize) {
        return false;
    }

    uint32_t block = bitmap->map[_bitToBlock(bit)];
    uint32_t blockMask = _bitToBlockMask(bit % WORDSIZE);

    return (block & blockMask) != 0;
}

void
bitmap_Set(Bitmap *bitmap, int bit)
{
    if (bit < 0 || bit > bitmap->bitSize) {
        return;
    }

    uint32_t blockMask = _bitToBlockMask(bit % WORDSIZE);
    bitmap->map[_bitToBlock(bit)] |= blockMask;
}

void
bitmap_SetVector(Bitmap *bitmap, Bitmap *other) {
    if (bitmap->bitSize != other->bitSize) {
        assertFalse(true, "Fatal error. Bitmaps were not the same size.");
    }
    for (int i = 0; i < bitmap->bitSize; i++) {
        bitmap->map[_bitToBlock(i)] |= ((other->map[_bitToBlock(i)] & _bitToBlockMask(i % WORDSIZE)));
    }
}

bool
bitmap_Contains(Bitmap *bitmap, Bitmap *other)
{
    if (bitmap->bitSize != other->bitSize) {
        assertFalse(true, "Fatal error. Bitmaps were not the same size.");
    }
    for (int i = 0; i < bitmap->bitSize; i++) {
        if (!bitmap_Get(bitmap, i) && bitmap_Get(other, i)) {
            return false;
        }
    }
    return true;
}

bool
bitmap_Equals(Bitmap *bitmap, Bitmap *other)
{
    if (bitmap->bitSize != other->bitSize) {
        assertFalse(true, "Fatal error. Bitmaps were not the same size.");
    }
    for (int i = 0; i < bitmap->bitSize; i++) {
        if (bitmap_Get(bitmap, i) != bitmap_Get(other, i)) {
            return false;
        }
    }
    return true;
}

void
bitmap_Clear(Bitmap *bitmap, int bit)
{
    if (bit < 0 || bit > bitmap->bitSize) {
        return;
    }
    bitmap->map[_bitToBlock(bit)] &= ~(_bitToBlockMask(bit % WORDSIZE));
}