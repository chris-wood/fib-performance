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

    PARCBitVector *vector1 = parcBitVector_Create();
    parcBitVector_Set(vector1, 2);
    PARCBitVector *vector2 = parcBitVector_Create();
    parcBitVector_Set(vector2, 13);
    PARCBitVector *vector3 = parcBitVector_Create();
    parcBitVector_Set(vector3, 2);
    parcBitVector_Set(vector3, 13);
    PARCBitVector *vector4 = parcBitVector_Create();
    parcBitVector_Set(vector4, 42);

    fib_Insert(fib, name1, vector1);

    PARCBitVector *result1 = fib_LPM(fib, name1);
    assertNotNull(result1, "Expected non NULL result");
    assertTrue(parcBitVector_Equals(result1, vector1), "Expected the exact match to be returned");
    parcBitVector_Release(&result1);

    PARCBitVector *result2 = fib_LPM(fib, name4);
    assertNotNull(result2, "Expected non NULL result");
    assertTrue(parcBitVector_Equals(result2, vector1), "Expected the first vector to be returned");
    parcBitVector_Release(&result2);

    PARCBitVector *result3 = fib_LPM(fib, name3);
    assertTrue(result3 == NULL, "Expected nothing to be found");

    parcBitVector_Release(&vector1);
    parcBitVector_Release(&vector2);
    parcBitVector_Release(&vector3);
    parcBitVector_Release(&vector4);

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
    Name *name1 = name_Hash(name1Orig, hasher);
    name_Destroy(&name1Orig);
    Name *name2Orig = name_CreateFromCString("ccnx:/a/b/a/d");
    Name *name2 = name_Hash(name2Orig, hasher);
    name_Destroy(&name2Orig);
    Name *name3Orig = name_CreateFromCString("ccnx:/");
    Name *name3 = name_Hash(name3Orig, hasher);
    name_Destroy(&name3Orig);
    Name *name4Orig = name_CreateFromCString("ccnx:/a/b/c/d/e");
    Name *name4 = name_Hash(name4Orig, hasher);
    name_Destroy(&name4Orig);

    PARCBitVector *vector1 = parcBitVector_Create();
    parcBitVector_Set(vector1, 2);
    PARCBitVector *vector2 = parcBitVector_Create();
    parcBitVector_Set(vector2, 13);
    PARCBitVector *vector3 = parcBitVector_Create();
    parcBitVector_Set(vector3, 2);
    parcBitVector_Set(vector3, 13);
    PARCBitVector *vector4 = parcBitVector_Create();
    parcBitVector_Set(vector4, 42);

//    fib_Insert(fib, name1, vector1);
//
//    PARCBitVector *result1 = fib_LPM(fib, name1);
//    assertNotNull(result1, "Expected non NULL result");
//    assertTrue(parcBitVector_Equals(result1, vector1), "Expected the exact match to be returned");
//    parcBitVector_Release(&result1);
//
//    PARCBitVector *result2 = fib_LPM(fib, name4);
//    assertNotNull(result2, "Expected non NULL result");
//    assertTrue(parcBitVector_Equals(result2, vector1), "Expected the first vector to be returned");
//    parcBitVector_Release(&result2);
//
//    PARCBitVector *result3 = fib_LPM(fib, name3);
//    assertTrue(result3 == NULL, "Expected nothing to be found");

    parcBitVector_Release(&vector1);
    parcBitVector_Release(&vector2);
    parcBitVector_Release(&vector3);
    parcBitVector_Release(&vector4);

    name_Destroy(&name1);
    name_Destroy(&name2);
    name_Destroy(&name3);
    name_Destroy(&name4);

    hasher_Destroy(&hasher);
}