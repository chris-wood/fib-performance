#include "../bitmap.h"

#include <LongBow/testing.h>
#include <LongBow/debugging.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>

#include <parc/testing/parc_MemoryTesting.h>

LONGBOW_TEST_RUNNER(bitmap)
{
    LONGBOW_RUN_TEST_FIXTURE(Core);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(bitmap)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(bitmap)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Core)
{
    LONGBOW_RUN_TEST_CASE(Core, bitmap_Create);
    LONGBOW_RUN_TEST_CASE(Core, bitmap_Stress);
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

LONGBOW_TEST_CASE(Core, bitmap_Create)
{
    Bitmap *map = bitmap_Create(256);
    assertNotNull(map, "Expected a non-NULL bitmap to be created");
    bitmap_Destroy(&map);
    assertNull(map, "Expected a NULL bitmap after bitmap_Destroy");
}

LONGBOW_TEST_CASE(Core, bitmap_Stress)
{
    Bitmap *map = bitmap_Create(256);
    assertNotNull(map, "Expected a non-NULL bitmap to be created");

    int primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 37, 41, 53, 59, 71, 73, 101, 233, 239, 241, 251};
    int numPrimes = sizeof(primes) / sizeof(int);

    for (int i = 0; i < numPrimes; i++) {
        int bit = primes[i];
        bitmap_Set(map, bit);
        assertTrue(bitmap_Get(map, bit), "Expected a set bit to be... well, set.");
    }

    for (int i = 0; i < 256; i++) {
        bool set = false;
        for (int p = 0; p < numPrimes; p++) {
            if (primes[p] == i) {
                set = true;
                break;
            }
        }
        if (!set) {
            assertFalse(bitmap_Get(map, i), "Expected a non-set bit to be false");
        }
    }

    for (int i = 0; i < numPrimes; i++) {
        int bit = primes[i];
        bitmap_Clear(map, bit);
        assertFalse(bitmap_Get(map, bit), "Expected a cleared bit to be... well, set.");
    }

    for (int i = 0; i < 256; i++) {
        assertFalse(bitmap_Get(map, i), "Expected bit %d to be cleared, but it was not.", i);
    }

    bitmap_Destroy(&map);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(bitmap);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
