#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_BufferComposer.h>
#include <parc/algol/parc_LinkedList.h>
#include <parc/algol/parc_Iterator.h>
#include <parc/developer/parc_StopWatch.h>

#include <stdio.h>
#include <ctype.h>

#include "fib.h"
#include "fib_naive.h"
#include "fib_cisco.h"

PARCBufferComposer *
readLine(FILE *fp)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();
    char curr = fgetc(fp);
    while ((isalnum(curr) || curr == ':' || curr == '/' || curr == '.' ||
            curr == '_' || curr == '(' || curr == ')' || curr == '[' ||
            curr == ']' || curr == '-' || curr == '%' || curr == '+' ||
            curr == '=' || curr == ';' || curr == '$' || curr == '\'') && curr != EOF) {
        parcBufferComposer_PutChar(composer, curr);
        curr = fgetc(fp);
    }
    return composer;
}

void usage() {
    fprintf(stderr, "usage: fib_perf <uri_file> <n> <alg>\n");
    fprintf(stderr, "   - uri_file = A file that contains a list of CCNx URIs\n");
    fprintf(stderr, "   - n        = The maximum length prefix to use when inserting names into the FIB\n");
    fprintf(stderr, "   - alg      = The FIB data structure to use: ['naive', 'cisco']\n");
}

// Rewrite this code as follows:
// 1. read list of names from file (single function)
// 2. create FIB load from list of names
// 3. insert FIB names into the FIB
// 4. lookup every name in the original name list

int
main(int argc, char **argv)
{
    if (argc != 4) {
        usage();
        exit(-1);
    }

    char *fname = argv[1];
    int N = atoi(argv[2]);
    char *alg = argv[3];

    FILE *file = fopen(fname, "r");
    if (file == NULL) {
        perror("Could not open file");
        usage();
        exit(-1);
    }

    // Create the FIB and list to hold all of the names
    PARCLinkedList *nameList = parcLinkedList_Create();
    PARCLinkedList *vectorList = parcLinkedList_Create();


    // Create the FIB table
    FIB *fib = NULL;
    if (strcmp(alg, "cisco") == 0) {
        FIBCisco *ciscoFIB = fibCisco_Create(3);
        fib = fib_Create(ciscoFIB, CiscoFIBAsFIB);
    } else if (strcmp(alg, "naive") == 0) {
        FIBNaive *nativeFIB = fibNative_Create();
        fib = fib_Create(nativeFIB, NativeFIBAsFIB);    
    } else {
        perror("Invalid algorithm specified\n");
        usage();
        exit(-1);
    }

    int num = 0;
    int index = 0;
    do {

        PARCBufferComposer *composer = readLine(file);
        PARCBuffer *bufferString = parcBufferComposer_ProduceBuffer(composer);
        if (parcBuffer_Remaining(bufferString) == 0) {
            break;
        }

        char *string = parcBuffer_ToString(bufferString);
        parcBufferComposer_Release(&composer);

        // Create the original name and store it for later
        CCNxName *name = ccnxName_CreateFromCString(string);
        char *nameString = ccnxName_ToString(name);
        printf("Read %d: %s\n", index, nameString);
        parcMemory_Deallocate(&nameString);
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
        fib_Insert(fib, copy, vector);
        parcLinkedList_Append(vectorList, vector);

        ccnxName_Release(&copy);
        parcBitVector_Release(&vector);

        index++;
    } while (true);

    PARCIterator *iterator = parcLinkedList_CreateIterator(nameList);
    index = 0;

    while (parcIterator_HasNext(iterator)) {

        CCNxName *name = parcIterator_Next(iterator);
        PARCBitVector *vector = parcBitVector_Create();

        PARCStopwatch *timer = parcStopwatch_Create();
        parcStopwatch_Start(timer);

        // Lookup and time it.
        uint64_t startTime = parcStopwatch_ElapsedTimeNanos(timer);
        PARCBitVector *output = fib_LPM(fib, name);
        uint64_t endTime = parcStopwatch_ElapsedTimeNanos(timer);

        PARCBitVector *expected = parcLinkedList_GetAtIndex(vectorList, index++);
        assertNotNull(output, "Expected a non-NULL output");
        //assertTrue(parcBitVector_Equals(output, expected), "Expected the correct return vector");

        uint64_t elapsedTime = endTime - startTime;
        printf("Time %d: %llu ns\n", index, elapsedTime);

        parcBitVector_Release(&vector);
        parcStopwatch_Release(&timer);
    }

    return 0;
}
