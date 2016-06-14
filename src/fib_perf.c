#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_BufferComposer.h>
#include <parc/algol/parc_LinkedList.h>
#include <parc/algol/parc_Iterator.h>
#include <parc/developer/parc_StopWatch.h>

#include <stdio.h>
#include <ctype.h>

#include "fib.h"

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
    fprintf(stderr, "usage: fib_perf <uri_file> <n>\n");
    fprintf(stderr, "   - uri_file = A file that contains a list of CCNx URIs\n");
    fprintf(stderr, "   - n        = The maximum length prefix to use when inserting names into the FIB\n");
}

int
main(int argc, char **argv)
{
    if (argc != 3) {
        usage();
        exit(-1);
    }

    char *fname = argv[1];
    int N = atoi(argv[2]);

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
    FIB *fib = fib_Create(FIBAlgorithm_Cisco, FIBMode_Hash);

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
    } while (index < N);

    PARCIterator *iterator = parcLinkedList_CreateIterator(nameList);
    index = 0;

    while (parcIterator_HasNext(iterator)) {

        CCNxName *name = parcIterator_Next(iterator);
        PARCBitVector *vector = parcBitVector_Create();

        PARCStopwatch *timer = parcStopwatch_Create();
        parcStopwatch_Start(timer);

        // Lookup and time it.
        uint64_t startTime = parcStopwatch_ElapsedTimeNanos(timer);
        PARCBitVector *output = fib_Lookup(fib, name);
        uint64_t endTime = parcStopwatch_ElapsedTimeNanos(timer);

        PARCBitVector *expected = parcLinkedList_GetAtIndex(vectorList, index++);
        assertNotNull(output, "Expected a non-NULL output");
        assertTrue(parcBitVector_Equals(output, expected), "Expected the correct return vector");

        uint64_t elapsedTime = endTime - startTime;
        printf("Time %d: %zu ns\n", index, elapsedTime);

        parcBitVector_Release(&vector);
        parcStopwatch_Release(&timer);
    }

    return 0;
}
