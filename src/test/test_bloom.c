#include "../bloom.c"

#include <LongBow/testing.h>
#include <LongBow/debugging.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>

#include <parc/testing/parc_MemoryTesting.h>

LONGBOW_TEST_RUNNER(bloom)
{
    LONGBOW_RUN_TEST_FIXTURE(Core);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(bloom)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(bloom)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Core)
{
    LONGBOW_RUN_TEST_CASE(Core, bloom_Create);
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

LONGBOW_TEST_CASE(Core, bloom_Create)
{
//    BF *bloom = bloom_CreateWithLinkedBuckets(bloomOverflowStrategy_OverflowBucket, true);
//    assertNotNull(bloom, "Expected a non-NULL bloom to be created");
//    bloom_Destroy(&bloom);
//    assertNull(bloom, "Expected a NULL bloom after bloom_Destroy");
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(bloom);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
