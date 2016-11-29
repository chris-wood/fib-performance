#include "../bloom.h"

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
    LONGBOW_RUN_TEST_CASE(Core, bloom_Add);
    LONGBOW_RUN_TEST_CASE(Core, bloom_AddTest);
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
    BloomFilter *bf = bloom_Create(128, 3);
    assertNotNull(bf, "Expected a non-NULL bloom to be created");
    bloom_Destroy(&bf);
    assertNull(bf, "Expected a NULL bloom after bloom_Destroy");
}

LONGBOW_TEST_CASE(Core, bloom_Add)
{
    BloomFilter *bf = bloom_Create(128, 3);

    PARCBuffer *x = parcBuffer_AllocateCString("foo");
    PARCBuffer *y = parcBuffer_AllocateCString("bar");
    PARCBuffer *z = parcBuffer_AllocateCString("baz");

    bloom_Add(bf, x);
    bloom_Add(bf, y);
    bloom_Add(bf, z);

    parcBuffer_Release(&x);
    parcBuffer_Release(&y);
    parcBuffer_Release(&z);

    bloom_Destroy(&bf);
}

LONGBOW_TEST_CASE(Core, bloom_AddTest)
{
    BloomFilter *bf = bloom_Create(128, 3);

    PARCBuffer *x = parcBuffer_AllocateCString("foo");
    PARCBuffer *y = parcBuffer_AllocateCString("bar");
    PARCBuffer *z = parcBuffer_AllocateCString("baz");

    bloom_Add(bf, x);
    assertTrue(bloom_Test(bf, x), "Item x not detected in the filter");

    bloom_Add(bf, y);
    assertTrue(bloom_Test(bf, y), "Item y not detected in the filter");

    // z is not in the filter
    bool inFilter = bloom_Test(bf, z);
    assertFalse(inFilter, "Item z detected in the filter when really it should not have been");

    parcBuffer_Release(&x);
    parcBuffer_Release(&y);
    parcBuffer_Release(&z);

    bloom_Destroy(&bf);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(bloom);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
