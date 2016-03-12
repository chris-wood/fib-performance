#include <ccnx/forwarder/athena/athena_FIB.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_LinkedList.h>
#include <parc/algol/parc_Iterator.h>
#include <stdio.h>

bool
readLine(FILE *fp, char **result, int *len)
{
    char *string = malloc(1);
    char curr = fgetc(fp);
    int number = 1;
    while (!(curr == '\n' || curr == '\r') && curr != EOF) {
        string = realloc(string, number + 1);
        string[number - 1] = curr;
        number++;
        curr = fgetc(fp);
    }

    *len = number;
    *result = string;

    if (curr == EOF) {
        return true;
    } else {
        return false;
    }
}

void usage() {
    fprintf(stderr, "usage: fib_perf <uri_file> <n>\n");
    fprintf(stderr, "   - uri_file = A file that contains a list of CCNx URIs\n");
    fprintf(stderr, "   - n        = The maximum length prefix to use when inserting names into the FIB\n");
}

int main(int argc, char **argv)
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
    AthenaFIB *fib = athenaFIB_Create();
    PARCLinkedList *nameList = parcLinkedList_Create();

    char *line = NULL;
    int length = 0;
    bool done = false;
    int num = 0;
    do {
        done = readLine(file, &line, &length);

        printf("Read: %s\n", line);

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
