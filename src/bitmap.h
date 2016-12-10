//
// Created by Christopher Wood on 12/9/16.
//

#ifndef FIB_PERF_BITMAP_H
#define FIB_PERF_BITMAP_H

#include <stdint.h>
#include <stdbool.h>

struct bitmap;
typedef struct bitmap Bitmap;

Bitmap *bitmap_Create(int size);
void bitmap_Destroy(Bitmap **bitmapP);

bool bitmap_Get(Bitmap *bitmap, int bit);
bool bitmap_Contains(Bitmap *bitmap, Bitmap *other);
bool bitmap_Equals(Bitmap *bitmap, Bitmap *other);
void bitmap_Set(Bitmap *bitmap, int bit);
void bitmap_SetVector(Bitmap *bitmap, Bitmap *other);
void bitmap_Clear(Bitmap *bitmap, int bit);

#endif //FIB_PERF_BITMAP_H
