//
// Created by caw on 11/29/16.
//

#include "../fib_cisco.h"

#include <LongBow/testing.h>
#include <LongBow/debugging.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>

#include <parc/testing/parc_MemoryTesting.h>

#include "test_fib.c"

LONGBOW_TEST_RUNNER(fibCisco)
{
    LONGBOW_RUN_TEST_FIXTURE(Core);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(fibCisco)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(fibCisco)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Core)
{
    LONGBOW_RUN_TEST_CASE(Core, fibCisco_Create);
    LONGBOW_RUN_TEST_CASE(Core, fibCisco_LookupSimple);
    LONGBOW_RUN_TEST_CASE(Core, fibCisco_LookupHashed);
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

LONGBOW_TEST_CASE(Core, fibCisco_Create)
{
//    BloomFilter *bf = fibCisco_Create(128, 3);
//    assertNotNull(bf, "Expected a non-NULL fibCisco to be created");
//    fibCisco_Destroy(&bf);
//    assertNull(bf, "Expected a NULL fibCisco after fibCisco_Destroy");
}

LONGBOW_TEST_CASE(Core, fibCisco_LookupSimple)
{
    FIBCisco *cisco = fibCisco_Create(3);
    assertNotNull(cisco, "Expected a non-NULL FIBCisco to be created");

    FIB *fib = fib_Create(cisco, CiscoFIBAsFIB);
    assertNotNull(fib, "Expected non-NULL FIB");

    test_fib_lookup(fib);

    fib_Destroy(&fib);
}

LONGBOW_TEST_CASE(Core, fibCisco_LookupHashed)
{
    FIBCisco *cisco = fibCisco_Create(3);
    assertNotNull(cisco, "Expected a non-NULL FIBCisco to be created");

    FIB *fib = fib_Create(cisco, CiscoFIBAsFIB);
    assertNotNull(fib, "Expected non-NULL FIB");

    test_fib_hash_lookup(fib);

    fib_Destroy(&fib);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(fibCisco);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
