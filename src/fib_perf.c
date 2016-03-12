#include <ccnx/forwarder/athena/athena_FIB.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_LinkedList.h>
#include <parc/algol/parc_Iterator.h>
#include <stdio.h>

bool
readLine(FILE *fp, char **result, int *len)
{
    *result = malloc(1);
    char curr = fgetc(fp);
    *len = 1;
    *result[0] = curr;
    while (curr != '\n' && curr != EOF) {
        result = realloc(*result, *len + 1);
        *result[*len] = curr;
        *len++;
        curr = fgetc(fp);
    }

    if (curr == EOF) {
        return false;
    } else {
        return true;
    }
}

int main(int argc, char **argv)
{
    AthenaFIB *fib = athenaFIB_Create();
    // CCNxName *testName1 = ccnxName_CreateFromURI("lci:/a/b/c");
    // CCNxName *testName2 = ccnxName_CreateFromURI("lci:/a/b/a");
    // CCNxName *testName3 = ccnxName_CreateFromURI("lci:/");
    //
    // PARCBitVector *testVector1 = parcBitVector_Create();
    // parcBitVector_Set(testVector1, 0);
    // PARCBitVector *testVector2 = parcBitVector_Create();
    // parcBitVector_Set(testVector2, 42);
    // PARCBitVector *testVector12 = parcBitVector_Create();
    // parcBitVector_Set(testVector12, 0);
    // parcBitVector_Set(testVector12, 42);
    // PARCBitVector *testVector3 = parcBitVector_Create();
    // parcBitVector_Set(testVector3, 23);
    //
    // athenaFIB_AddRoute(fib, testName1, testVector1);
    // TODO: add the others later...
    // printf("added.\n");

    char *fname = argv[1];
    int N = atoi(argv[2]);

    PARCLinkedList *nameList = parcLinkedList_Create();

    FILE *file = fopen(fname, "r");
    char *line = NULL;
    int length = 0;
    bool done = false;
    int num = 0;
    do {
        done = readLine(file, &line, &length);

        // Create the original name and store it for later
        CCNxName *name = ccnxName_CreateFromCString(line);
        parcLinkedList_Append(nameList, name);

        // Truncate to N components if necessary
        CCNxName *copy = ccnxName_Copy(name);
        int numComponents = ccnxName_GetSegmentCount(copy);
        if (N < numComponents) {
            copy = ccnxName_Trim(copy, numComponents - N);
        }

        PARCBitVector *vector = parcBitVector_Create();
        parcBitVector_Set(vector, num);
        num = (num + 1) % 10;

        // Insert into the FIB
        athenaFIB_AddRoute(fib, copy, vector);

        ccnxName_Release(&copy);
        parcBitVector_Release(&vector);

        free(line);
    } while (!done);

    PARCIterator *iterator = parcLinkedList_CreateIterator(nameList);

    struct timeval start, end;
    gettimeofday(&start, NULL);

    while (parcIterator_HasNext(iterator)) {
        CCNxName *name = parcIterator_Next(iterator);
        // Lookup and time it.
    }

    gettimeofday(&end, NULL);
    printf ("Total time = %f seconds\n", (double) (end.tv_usec - start.tv_usec) / 1000000 + (double) (end.tv_sec - start.tv_sec));

    athenaFIB_Release(&fib);

    // ccnxName_Release(&testName1);
    // ccnxName_Release(&testName2);
    // ccnxName_Release(&testName3);
    // parcBitVector_Release(&testVector1);
    // parcBitVector_Release(&testVector2);
    // parcBitVector_Release(&testVector12);
    // parcBitVector_Release(&testVector3);

    return 0;
}
