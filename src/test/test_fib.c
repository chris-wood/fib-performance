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
    Name *name3 = name_CreateFromCString("ccnx:/a");
    Name *name4 = name_CreateFromCString("ccnx:/a/b/c/d/e");

    Name *name5 = name_CreateFromCString("ccnx:/0815/org");
    Name *name6 = name_CreateFromCString("ccnx:/0815/org/30XKPUEWDFW3L0JPP2MZXV2ZVBD/E9MKBV8YWQRL9OLYF/E0WF90IOP8H1XL2S3AMA1AF9ZQ59/N4509KF805AKM49VO4HLUTWC/UWT5X9RIQCBB/FIR5HGH3YO491VZJDYBR5M/5LGV7DAYBV6SKSH6S9AK54FLX5OEMR/J7W1D5QONHEJYMT0F0KN/7SAOMX3W42VV0VATK/23P1CTILY7774842MBIR0FJCL5WKV9ZHKS2COZ4Q9OU22M4INJJXEZRH/R3L103R7Y8019F8FBESNI/9GBVN73OGN2C9NPAW4EMDALZGFXPVZMMK33V/W8MN75V81J59FLI3WZFWOQ5I02VNEZI6SYTDUXETE03205NBHZ6LTE6VVWN0HECQ02VM/AJ06WE67B/B5M42OBAD147GZCC6GBKAV21Q38L/4I1RZ1DCN/BR1IMVR8HS68ZX/NGUMXVNWM6P9XWKF489WKJUJH7I8HU9EQ8X9T222GUT/U4/IQWTYX5ZPSZE89IXVT/I74KJ2SY/DK2TKU69F0PHY1J88/QA9VR4EUFVY1LQ042WEV2NE0PE9Q3DX//LY8LDMAAEVLA6KLA8FCJ8GTQVNEZ79C6JE6ONYRQZDEA/RWQ4OFFP6T32XP6N44DSGOYVQYIBCHLI4NCO4/XRVNGOR3LY/ASQ4OHUY0CYN2KEDUNXKNMMB4OYDULLUGEJ1UWEW/ED4NAXOHI8VE43JBAPRPO/E5C2OP02460EOKOWMSCGKL47ZDF2R4UD77W8D6KNE/2JVHFPRSJ3CSM04WB/32PQ7LSYV7A40X183R5/K45LL61/02IPFPD4BL0K6XGG49JYUBA4X9FWVOFQ1POBORKYNCCM9QRY2BA/FBEIGYS4ETA2VE7XOQ/0QNH1H8/19PK9OPXP7DE4ZXE3CSLBSMH5DJNFAQPLVBTES99FQK77ASJLCUV4CTIBIMCOHFCW");

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

    fib_Insert(fib, name5, vector2);
    Bitmap *result4 = fib_LPM(fib, name6);
    assertNotNull(result4, "Expected non NULL result");
    assertTrue(bitmap_Equals(result4, vector2), "Expected the exact match to be returned");

    bitmap_Destroy(&vector1);
    bitmap_Destroy(&vector2);
    bitmap_Destroy(&vector3);
    bitmap_Destroy(&vector4);

    name_Destroy(&name1);
    name_Destroy(&name2);
    name_Destroy(&name3);
    name_Destroy(&name4);
    name_Destroy(&name5);
    name_Destroy(&name6);
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
