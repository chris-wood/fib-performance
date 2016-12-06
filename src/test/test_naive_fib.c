//
// Created by caw on 11/29/16.
//

#include "../fib_naive.h"

#include <LongBow/testing.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>

#include <parc/testing/parc_MemoryTesting.h>

#include "test_fib.c"

LONGBOW_TEST_RUNNER(fibNaive)
{
    LONGBOW_RUN_TEST_FIXTURE(Core);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(fibNaive)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(fibNaive)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Core)
{
    LONGBOW_RUN_TEST_CASE(Core, fibNaive_Create);
    LONGBOW_RUN_TEST_CASE(Core, fibNaive_LookupSimple);
    LONGBOW_RUN_TEST_CASE(Core, fibNaive_LookupHashed);
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

LONGBOW_TEST_CASE(Core, fibNaive_Create)
{
    FIBNaive *fib = fibNative_Create();
    assertNotNull(fib, "Expected a non-NULL fibNaive to be created");
    fibNaive_Destroy(&fib);
}

LONGBOW_TEST_CASE(Core, fibNaive_LookupSimple)
{
    FIBNaive *native = fibNative_Create();
    assertNotNull(native, "Expected a non-NULL fibNaive to be created");

    FIB *fib = fib_Create(native, NativeFIBAsFIB);
    assertNotNull(fib, "Expected non-NULL FIB");

    test_fib_lookup(fib);

    fib_Destroy(&fib);
}

LONGBOW_TEST_CASE(Core, fibNaive_LookupHashed)
{
    FIBNaive *native = fibNative_Create();
    assertNotNull(native, "Expected a non-NULL fibNaive to be created");

    FIB *fib = fib_Create(native, NativeFIBAsFIB);
    assertNotNull(fib, "Expected non-NULL FIB");

    test_fib_hash_lookup(fib);

    fib_Destroy(&fib);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(fibNaive);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
