#include <time.h>
#include <stdio.h>
#include <ctype.h>
#include <getopt.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_BufferComposer.h>
#include <parc/algol/parc_LinkedList.h>
//#include <parc/algol/parc_Iterator.h>
//#include <parc/developer/parc_Stopwatch.h>

#include "fib.h"
#include "fib_naive.h"
#include "fib_cisco.h"
#include "fib_caesar.h"
#include "fib_caesar_filter.h"

struct timespec
timer_start()
{
    struct timespec start_time;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time);
    return start_time;
}

long
timer_end(struct timespec start_time)
{
    struct timespec end_time;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);
    long diffInNanos = ((end_time.tv_sec - start_time.tv_sec) * 10000000000L) + (end_time.tv_nsec - start_time.tv_nsec);
    return diffInNanos;
}

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
    fprintf(stderr, "   - uri_file  = A file that contains a list of CCNx URIs\n");
    fprintf(stderr, "   - n         = The maximum length prefix to use when inserting names into the FIB\n");
    fprintf(stderr, "   - alg       = The FIB data structure to use: ['naive', 'cisco']\n");
}

typedef struct {
    char *uriFile;
    FIB *fib;
    uint32_t maxNameLength;
    bool preProcessed;
} FIBOptions;

FIBOptions *
parseCommandLineOptions(int argc, char **argv)
{
    static struct option longopts[] = {
            { "uri_file",   required_argument,  NULL, 'f' },
            { "n",          required_argument,  NULL, 'n' },
            { "alg",        required_argument,  NULL, 'a' },
            { "pre_processed", no_argument, NULL, 'p'},
            { "help",       no_argument,        NULL, 'h'},
            { NULL,0,NULL,0}
    };

    if (argc < 5) {
        usage();
        exit(EXIT_FAILURE);
    }

    FIBOptions *options = malloc(sizeof(FIBOptions));
    options->uriFile = NULL;
    options->fib = NULL;
    options->maxNameLength = 0;
    options->preProcessed = false;

    int c;
    while (optind < argc) {
        if ((c = getopt_long(argc, argv, "phf:n:a:", longopts, NULL)) != -1) {
            switch(c) {
                case 'p':
                    options->preProcessed = true;
                    break;
                case 'f':
                    options->uriFile = malloc(strlen(optarg));
                    strcpy(options->uriFile, optarg);
                    break;
                case 'n':
                    sscanf(optarg, "%u", &(options->maxNameLength));
                    break;
                case 'a': {
                    FIB *fib = NULL;
                    if (strcmp(optarg, "cisco") == 0) {
                        FIBCisco *ciscoFIB = fibCisco_Create(3);
                        fib = fib_Create(ciscoFIB, CiscoFIBAsFIB);
                    } else if (strcmp(optarg, "naive") == 0) {
                        FIBNaive *nativeFIB = fibNative_Create();
                        fib = fib_Create(nativeFIB, NativeFIBAsFIB);
                    } else if (strcmp(optarg, "caesar") == 0) {
                        FIBCaesar *caesarFIB = fibCaesar_Create(128, 128, 5);
                        fib = fib_Create(caesarFIB, CaesarFIBAsFIB);
                    } else if (strcmp(optarg, "caesar-filter") == 0) {
                        FIBCaesarFilter *filterFIB = fibCaesarFilter_Create(256, 128, 128, 5);
                        fib = fib_Create(filterFIB, CaesarFilterFIBAsFIB);
                    } else {
                        perror("Invalid algorithm specified\n");
                        usage();
                        exit(EXIT_FAILURE);
                    }
                    options->fib = fib;
                    break;
                }
                case 'h':
                    usage();
                    exit(EXIT_SUCCESS);
                default:
                    break;
            }
        }
    }

    return options;
}

struct name_list_node;
typedef struct name_list_node {
    struct name_list_node *next;
    Name *name;
} NameList;

static NameList *
_createNameList(Name *name)
{
    NameList *list = (NameList *) malloc(sizeof(NameList));
    if (list != NULL) {
        list->name = name;
        list->next = NULL;
    }
    return list;
}


// Rewrite this code as follows:
// 1. read list of names from file (single function)
// 2. create FIB load from list of names
// 3. insert FIB names into the FIB
// 4. lookup every name in the original name list

int
main(int argc, char **argv)
{
    FIBOptions *options = parseCommandLineOptions(argc, argv);

    FILE *file = fopen(options->uriFile, "r");
    if (file == NULL) {
        perror("Could not open file");
        usage();
        exit(EXIT_FAILURE);
    }

    // Create the FIB and list to hold all of the names
    NameList *list = NULL;
    NameList *head = NULL;
    PARCLinkedList *vectorList = parcLinkedList_Create();

    // Extract options
    FIB *fib = options->fib;
//    int N = options->maxNameLength;

    int num = 0;
    int index = 0;
    do {
        PARCBufferComposer *composer = readLine(file);
        PARCBuffer *bufferString = parcBufferComposer_ProduceBuffer(composer);
        parcBufferComposer_Release(&composer);
        if (parcBuffer_Remaining(bufferString) == 0) {
            break;
        }

        // Create the original name and store it for later
        char *string = parcBuffer_ToString(bufferString);
        printf("Parsing: %s\n", string);
        Name *name = name_CreateFromCString(string);
        PARCBuffer *wireFormat = name_GetWireFormat(name, name_GetSegmentCount(name));
        char *nameString = parcBuffer_ToHexString(wireFormat);
        printf("Read %d: %s\n", index, nameString);
        parcMemory_Deallocate(&nameString);
        parcBuffer_Release(&wireFormat);

        // Add the name to the name list
        if (list == NULL) {
            list = _createNameList(name);
            head = list;
        } else {
            NameList *next = _createNameList(name);
            list->next = next;
            list = next;
        }

        // Truncate to N components if necessary
//        CCNxName *copy = ccnxName_Copy(name);
//        int numComponents = ccnxName_GetSegmentCount(copy);
//        if (N < numComponents) {
//            copy = ccnxName_Trim(copy, numComponents - N);
//        }

        PARCBitVector *vector = parcBitVector_Create();
        assertNotNull(vector, "Could not allocate a PARCBitVector");
        parcBitVector_Set(vector, num);
        num = (num + 1) % 10; // 10 is not special -- the bit vectors don't actually matter here...

        // Insert into the FIB
        fib_Insert(fib, name, vector);
        parcLinkedList_Append(vectorList, vector);

        parcBitVector_Release(&vector);

        index++;
    } while (true);

    index = 0;

    NameList *curr = head;
    while (curr != NULL) {
        Name *name = curr->name;
        PARCBitVector *vector = parcBitVector_Create();

        // Lookup and time it.
        struct timespec start = timer_start();
        PARCBitVector *output = fib_LPM(fib, name);
        long elapsedTime = timer_end(start);

        assertNotNull(output, "Expected a non-NULL output");
//        PARCBitVector *expected = parcLinkedList_GetAtIndex(vectorList, index++);
        //assertTrue(parcBitVector_Equals(output, expected), "Expected the correct return vector");

        printf("Time %d: %lu ns\n", index, elapsedTime);

        parcBitVector_Release(&vector);

        index++;
        curr = curr->next;
    }

    return EXIT_SUCCESS;
}
