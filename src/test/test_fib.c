//
// Created by caw on 11/29/16.
//

#include <LongBow/testing.h>
#include <LongBow/debugging.h>

#include "../sha256hasher.h"

void test_fib_lookup(FIB *fib)
{
    // Name3 (the default route) contains both vector1 and vector2.
    // Name1 (a/b/c) contains only vector 1.
    // Name4 (a/b/c/d) is what we're looking for.
    Name *name1 = name_CreateFromCString("ccnx:/a/b/c/d");
    Name *name2 = name_CreateFromCString("ccnx:/a/b/a/d");
    Name *name3 = name_CreateFromCString("ccnx:/");
    Name *name4 = name_CreateFromCString("ccnx:/a/b/c/d/e");

    Bitmap *vector1 = bitmap_Create(128);
    bitmap_Set(vector1, 2);
    Bitmap *vector2 = bitmap_Create(128);
    bitmap_Set(vector2, 13);
    Bitmap *vector3 = bitmap_Create(128);
    bitmap_Set(vector3, 2);
    bitmap_Set(vector3, 13);
    Bitmap *vector4 = bitmap_Create(128);
    bitmap_Set(vector4, 19);

    fib_Insert(fib, name1, vector1);

    Bitmap *result1 = fib_LPM(fib, name1);
    assertNotNull(result1, "Expected non NULL result");
    assertTrue(bitmap_Equals(result1, vector1), "Expected the exact match to be returned");

    Bitmap *result2 = fib_LPM(fib, name4);
    assertNotNull(result2, "Expected non NULL result");
    assertTrue(bitmap_Equals(result2, vector1), "Expected the first vector to be returned");

    Bitmap *result3 = fib_LPM(fib, name3);
    assertTrue(result3 == NULL, "Expected nothing to be found");

    bitmap_Destroy(&vector1);
    bitmap_Destroy(&vector2);
    bitmap_Destroy(&vector3);
    bitmap_Destroy(&vector4);

    name_Destroy(&name1);
    name_Destroy(&name2);
    name_Destroy(&name3);
    name_Destroy(&name4);
}

void test_fib_hash_lookup(FIB *fib)
{
    // XXX: hasher...
    SHA256Hasher *nameHasher = sha256hasher_Create();
    Hasher *hasher = hasher_Create(nameHasher, SHA256HashAsHasher);

    // Name3 (the default route) contains both vector1 and vector2.
    // Name1 (a/b/c) contains only vector 1.
    // Name4 (a/b/c/d) is what we're looking for.
    Name *name1Orig = name_CreateFromCString("ccnx:/a/b/c/d");
    Name *name1 = name_Hash(name1Orig, hasher, 8);
    name_Destroy(&name1Orig);
    Name *name2Orig = name_CreateFromCString("ccnx:/a/b/a/d");
    Name *name2 = name_Hash(name2Orig, hasher, 8);
    name_Destroy(&name2Orig);
    Name *name3Orig = name_CreateFromCString("ccnx:/");
    Name *name3 = name_Hash(name3Orig, hasher, 8);
    name_Destroy(&name3Orig);
    Name *name4Orig = name_CreateFromCString("ccnx:/a/b/c/d/e");
    Name *name4 = name_Hash(name4Orig, hasher, 8);
    name_Destroy(&name4Orig);

    Bitmap *vector1 = bitmap_Create(128);
    bitmap_Set(vector1, 2);
    Bitmap *vector2 = bitmap_Create(128);
    bitmap_Set(vector2, 13);
    Bitmap *vector3 = bitmap_Create(128);
    bitmap_Set(vector3, 2);
    bitmap_Set(vector3, 13);
    Bitmap *vector4 = bitmap_Create(128);
    bitmap_Set(vector4, 42);

    fib_Insert(fib, name1, vector1);

    Bitmap *result1 = fib_LPM(fib, name1);
    assertNotNull(result1, "Expected non NULL result");
    assertTrue(bitmap_Equals(result1, vector1), "Expected the exact match to be returned");

    Bitmap *result2 = fib_LPM(fib, name4);
    assertNotNull(result2, "Expected non NULL result");
    assertTrue(bitmap_Equals(result2, vector1), "Expected the first vector to be returned");

    Bitmap *result3 = fib_LPM(fib, name3);
    assertTrue(result3 == NULL, "Expected nothing to be found");

    bitmap_Destroy(&vector1);
    bitmap_Destroy(&vector2);
    bitmap_Destroy(&vector3);
    bitmap_Destroy(&vector4);

    name_Destroy(&name1);
    name_Destroy(&name2);
    name_Destroy(&name3);
    name_Destroy(&name4);

    hasher_Destroy(&hasher);
}
