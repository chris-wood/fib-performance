#include "../prefix_bloom.c"

#include <LongBow/testing.h>
#include <LongBow/debugging.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>

#include <parc/testing/parc_MemoryTesting.h>

LONGBOW_TEST_RUNNER(prefixBloom)
{
    LONGBOW_RUN_TEST_FIXTURE(Core);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(prefixBloom)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(prefixBloom)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Core)
{
    LONGBOW_RUN_TEST_CASE(Core, prefixBloom_Create);
    LONGBOW_RUN_TEST_CASE(Core, prefixBloom_Add);
    LONGBOW_RUN_TEST_CASE(Core, prefixBloom_AddTest);
}

LONGBOW_TEST_FIXTURE_SETUP(Core)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Core)
{
    if (!parcMemoryTesting_ExpectedOutstanding(0, "%s mismanaged memory.", longBowTestCase_GetFullName(testCase))) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Core, prefixBloom_Create)
{
    PrefixBloomFilter *bf = prefixBloomFilter_Create(10, 128, 3);
    assertNotNull(bf, "Expected a non-NULL prefixBloom to be created");
    prefixBloomFilter_Destroy(&bf);
    assertNull(bf, "Expected a NULL prefixBloom after prefixBloom_Destroy");
}

LONGBOW_TEST_CASE(Core, prefixBloom_Add)
{
    PrefixBloomFilter *bf = prefixBloomFilter_Create(10, 128, 3);

    Name *x = name_CreateFromCString("ccnx:/foo");
    Name *y = name_CreateFromCString("ccnx:/bar");
    Name *z = name_CreateFromCString("ccnx:/baz");

    prefixBloomFilter_Add(bf, x);
    prefixBloomFilter_Add(bf, y);
    prefixBloomFilter_Add(bf, z);

    name_Destroy(&x);
    name_Destroy(&y);
    name_Destroy(&z);

    prefixBloomFilter_Destroy(&bf);
}

LONGBOW_TEST_CASE(Core, prefixBloom_AddTest)
{
    PrefixBloomFilter *bf = prefixBloomFilter_Create(10, 128, 3);

    Name *x1 = name_CreateFromCString("ccnx:/foo/barr");
    Name *x2 = name_CreateFromCString("ccnx:/foob/barr");
    Name *x3 = name_CreateFromCString("ccnx:/fo");
    Name *y = name_CreateFromCString("ccnx:/foo");
    Name *z = name_CreateFromCString("ccnx:/baz");

    // don't add X
    prefixBloomFilter_Add(bf, y);
    prefixBloomFilter_Add(bf, z);

    int index = prefixBloomFilter_LPM(bf, x1);
    assertTrue(index == 1, "Expected x to match only 1 segment, got %d", index);

    index = prefixBloomFilter_LPM(bf, x2);
    assertTrue(index == -1, "Expected x to match no segments, got %d", index);

    index = prefixBloomFilter_LPM(bf, x3);
    assertTrue(index == -1, "Expected x to match no segments, got %d", index);

    name_Destroy(&x1);
    name_Destroy(&x2);
    name_Destroy(&x3);
    name_Destroy(&y);
    name_Destroy(&z);

    prefixBloomFilter_Destroy(&bf);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(prefixBloom);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
