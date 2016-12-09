#include <time.h>
#include <stdio.h>
#include <ctype.h>
#include <getopt.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_BufferComposer.h>
#include <parc/statistics/parc_BasicStats.h>

#include "fib.h"
#include "fib_naive.h"
#include "fib_cisco.h"
#include "fib_caesar.h"
#include "fib_caesar_filter.h"
#include "sha256hasher.h"

#define DEFAULT_NUM_PORTS 256

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

typedef struct timed_result {
    long time;
    struct timed_result *next;
} TimedResult;

typedef struct {
    TimedResult *head;
    TimedResult *current;
} TimedResultSet;

static void
_appendTimedResult(TimedResultSet *set, long time)
{
    TimedResult *result = (TimedResult *) malloc(sizeof(TimedResult));
    result->time = time;
    result->next = NULL;

    if (set->head == NULL) {
        set->head = result;
        set->current = result;
    } else {
        set->current->next = result;
        set->current = result;
    }
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
    fprintf(stderr, "usage: fib_perf\n");
    fprintf(stderr, "   - load_file = A file that contains names to load the FIB\n");
    fprintf(stderr, "   - test_file = A file that contains names to pump through and test the FIB\n");
    fprintf(stderr, "   - n         = The maximum length prefix to use when inserting names into the FIB\n");
    fprintf(stderr, "   - alg       = The FIB data structure to use: ['naive', 'cisco', 'caesar', 'caesar-filter']\n");
    fprintf(stderr, "   - ports     = The number of ports supported\n");
    fprintf(stderr, "   - digest    = A flag to indicate that names should be hashed. Currently, SHA256 is used.\n");
}

typedef struct {
    char *loadFile;
    char *testFile;
    FIB *fib;
    Hasher *hasher;
    int numPorts;
    uint32_t maxNameLength;
} FIBOptions;

FIBOptions *
parseCommandLineOptions(int argc, char **argv)
{
    static struct option longopts[] = {
            { "load_file",   required_argument,  NULL, 'l' },
            { "test_file",   required_argument,  NULL, 't' },
            { "n",           required_argument,  NULL, 'n' },
            { "alg",         required_argument,  NULL, 'a' },
            { "num_ports",   required_argument,  NULL, 'p'},
            { "digest",      no_argument, NULL,  'd'},
            { "help",        no_argument,        NULL, 'h'},
            { NULL,0,NULL,0}
    };

    if (argc < 5) {
        usage();
        exit(EXIT_FAILURE);
    }

    FIBOptions *options = malloc(sizeof(FIBOptions));
    options->loadFile = NULL;
    options->testFile = NULL;
    options->fib = NULL;
    options->maxNameLength = 0;
    options->hasher = NULL;
    options->numPorts = DEFAULT_NUM_PORTS;

    int c;
    while (optind < argc) {
        if ((c = getopt_long(argc, argv, "hl:t:n:a:dp:", longopts, NULL)) != -1) {
            switch(c) {
                case 'l':
                    options->loadFile = malloc(strlen(optarg) + 1);
                    strcpy(options->loadFile, optarg);
                    break;
                case 't':
                    printf("parsing....\n");
                    options->testFile = malloc(strlen(optarg) + 1);
                    strcpy(options->testFile, optarg);
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
                case 'p': {
                    options->numPorts = atoi(optarg);
                }
                case 'd': {
                    SHA256Hasher *hasher = sha256hasher_Create();
                    options->hasher = hasher_Create(hasher, SHA256HashAsHasher);
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

static TimedResultSet *
_loadFIB(FIBOptions *options)
{
    TimedResultSet *timeResults = (TimedResultSet *) malloc(sizeof(TimedResultSet));
    assertTrue(timeResults != NULL, "Failed to allocate a result set. Fail immediately");
    timeResults->head = NULL;

    fprintf(stderr, "Loading from: %s\n", options->loadFile);
    FILE *file = fopen(options->loadFile, "r");
    if (file == NULL) {
        perror("Could not open load file");
        usage();
        exit(EXIT_FAILURE);
    }

    // This is the FIB to use
    FIB *fib = options->fib;

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
        printf("Read %d: %s\n", index, name_GetNameString(name));

        if (options->hasher != NULL) {
            Name *newName = name_Hash(name, options->hasher);
            name_Destroy(&name);
            name = newName;
        }

        PARCBitVector *vector = parcBitVector_Create();
        assertNotNull(vector, "Could not allocate a PARCBitVector");
        parcBitVector_Set(vector, num);
        num = (num + 1) % options->numPorts;

        // Insert into the FIB
        struct timespec start = timer_start();
        fib_Insert(fib, name, vector);
        long elapsedTime = timer_end(start);

        // Record the insertion time
        _appendTimedResult(timeResults, elapsedTime);

        index++;
    } while (true);

    return timeResults;
}

static TimedResultSet *
_testFIB(FIBOptions *options)
{
    TimedResultSet *timeResults = (TimedResultSet *) malloc(sizeof(TimedResultSet));
    assertTrue(timeResults != NULL, "Failed to allocate a result set. Fail immediately");
    timeResults->head = NULL;

    fprintf(stderr, "Testing from: %s\n", options->testFile);
    FILE *file = fopen(options->testFile, "r");
    if (file == NULL) {
        perror("Could not open test file");
        usage();
        exit(EXIT_FAILURE);
    }

    // This is the FIB to use
    FIB *fib = options->fib;

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
        printf("Read %d: %s\n", index, name_GetNameString(name));

        if (options->hasher != NULL) {
            Name *newName = name_Hash(name, options->hasher);
            name_Destroy(&name);
            name = newName;
        }

        // Look up the FIB and time it.
        struct timespec start = timer_start();
        PARCBitVector *output = fib_LPM(fib, name);
        long elapsedTime = timer_end(start);
        assertNotNull(output, "Expected a non-NULL output");

        // Record the insertion time
        _appendTimedResult(timeResults, elapsedTime);

        index++;
    } while (true);

    return timeResults;
}

int
main(int argc, char **argv)
{
    FIBOptions *options = parseCommandLineOptions(argc, argv);

    // Run the test
    TimedResultSet *insertionResults = _loadFIB(options);
    TimedResultSet *testResults = _testFIB(options);

    // Display the results
    PARCBasicStats *testStats = parcBasicStats_Create();
    TimedResult *curr = testResults->head;
    while (curr != NULL) {
        parcBasicStats_Update(testStats, (double) curr->time);
        curr = curr->next;
    }

    printf("mean : %f\n", parcBasicStats_Mean(testStats));
    printf("stdev: %f\n", parcBasicStats_StandardDeviation(testStats));

    return EXIT_SUCCESS;
}
