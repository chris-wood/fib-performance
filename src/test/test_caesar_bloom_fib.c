//
// Created by caw on 11/29/16.
//

#include "../fib_caesar_filter.h"

#include <LongBow/testing.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>

#include <parc/testing/parc_MemoryTesting.h>

#include "test_fib.c"

LONGBOW_TEST_RUNNER(fibCaesarFilter)
{
    LONGBOW_RUN_TEST_FIXTURE(Core);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(fibCaesarFilter)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(fibCaesarFilter)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Core)
{
    LONGBOW_RUN_TEST_CASE(Core, fibCaesarFilter_Create);
    LONGBOW_RUN_TEST_CASE(Core, fibCaesarFilter_LookupSimple);
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

LONGBOW_TEST_CASE(Core, fibCaesarFilter_Create)
{
//    BloomFilter *bf = fibCaesarFilter_Create(128, 3);
//    assertNotNull(bf, "Expected a non-NULL fibCaesarFilter to be created");
//    fibCaesarFilter_Destroy(&bf);
//    assertNull(bf, "Expected a NULL fibCaesarFilter after fibCaesarFilter_Destroy");
}

LONGBOW_TEST_CASE(Core, fibCaesarFilter_LookupSimple)
{
    FIBCaesarFilter *cisco = fibCaesarFilter_Create(128, 128, 128, 5);
    assertNotNull(cisco, "Expected a non-NULL FIBCisco to be created");

    FIB *fib = fib_Create(cisco, CaesarFilterFIBAsFIB);
    assertNotNull(fib, "Expected non-NULL FIB");

    test_fib_lookup(fib);

    fib_Destroy(&fib);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(fibCaesarFilter);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
