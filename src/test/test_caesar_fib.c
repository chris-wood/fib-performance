//
// Created by caw on 11/29/16.
//

#include "../fib_caesar.h"

#include <LongBow/testing.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>

#include <parc/testing/parc_MemoryTesting.h>

#include "test_fib.c"

LONGBOW_TEST_RUNNER(fibCaesar)
{
    LONGBOW_RUN_TEST_FIXTURE(Core);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(fibCaesar)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(fibCaesar)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Core)
{
    LONGBOW_RUN_TEST_CASE(Core, fibCaesar_Create);
    LONGBOW_RUN_TEST_CASE(Core, fibCaesar_LookupSimple);
    LONGBOW_RUN_TEST_CASE(Core, fibCaesar_LookupHashed);
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

LONGBOW_TEST_CASE(Core, fibCaesar_Create)
{
//    BloomFilter *bf = fibCaesar_Create(128, 3);
//    assertNotNull(bf, "Expected a non-NULL fibCaesar to be created");
//    fibCaesar_Destroy(&bf);
//    assertNull(bf, "Expected a NULL fibCaesar after fibCaesar_Destroy");
}

LONGBOW_TEST_CASE(Core, fibCaesar_LookupSimple)
{
    FIBCaesar *cisco = fibCaesar_Create(100, 128, 3);
    assertNotNull(cisco, "Expected a non-NULL FIBCaesar to be created");

    FIB *fib = fib_Create(cisco, CaesarFIBAsFIB);
    assertNotNull(fib, "Expected non-NULL FIB");

    test_fib_lookup(fib);

    fib_Destroy(&fib);
}

LONGBOW_TEST_CASE(Core, fibCaesar_LookupHashed)
{
    FIBCaesar *cisco = fibCaesar_Create(100, 128, 3);
    assertNotNull(cisco, "Expected a non-NULL FIBCaesar to be created");

    FIB *fib = fib_Create(cisco, CaesarFIBAsFIB);
    assertNotNull(fib, "Expected non-NULL FIB");

    test_fib_hash_lookup(fib);

    fib_Destroy(&fib);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(fibCaesar);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
