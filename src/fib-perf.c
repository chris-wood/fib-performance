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
#include "fib_merged_filter.h"
#include "sha256hasher.h"
#include "timer.h"
#include "bitmap.h"
#include "fib_patricia.h"
#include "fib_tbf.h"

#define DEFAULT_NUM_PORTS 256
#define DEFAULT_NUM_FILTERS 2 // from Caesar paper
#define DEFAULT_FILTER_SIZE 128

typedef struct timed_result {
    long time;
    struct timed_result *next;
} TimedResult;

typedef struct {
    int count;
    int total;
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
    set->total++;
}

static void
_addCount(TimedResultSet *set, int count)
{
    set->count = count;
}

static PARCBufferComposer *
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
    fprintf(stderr, "   - filters   = The number of filters to use for BF-based FIBs\n");
    fprintf(stderr, "   - n         = The maximum length prefix to use when inserting names into the FIB\n");
    fprintf(stderr, "   - alg       = The FIB data structure to use: ['naive', 'cisco', 'caesar', 'caesar-filter', 'merged-bf']\n");
    fprintf(stderr, "   - ports     = The number of ports supported\n");
    fprintf(stderr, "   - digest    = A flag to indicate that names should be hashed. Currently, SHA256 is used.\n");
}

typedef struct {
    char *loadFile;
    char *testFile;
    FIB *fib;
    Hasher *hasher;
    int hashSize;
    int numPorts;
    int numFilters;
    int filterSize;
    int trieDepth;
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
            { "filters",     required_argument,  NULL, 'f' },
            { "trie_depth",  required_argument,  NULL, 'x' },
            { "filter_size", required_argument,  NULL, 's' },
            { "num_ports",   required_argument,  NULL, 'p'},
            { "digest",      required_argument,  NULL, 'd'},
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
    options->hashSize = 0;
    options->hasher = NULL;
    options->numPorts = DEFAULT_NUM_PORTS;
    options->numFilters = DEFAULT_NUM_FILTERS;
    options->filterSize = DEFAULT_FILTER_SIZE;
    options->trieDepth = 2;

    int c;
    while (optind < argc) {
        if ((c = getopt_long(argc, argv, "hl:t:n:a:d:p:f:x:s:", longopts, NULL)) != -1) {
            switch(c) {
                case 'l':
                    options->loadFile = malloc(strlen(optarg) + 1);
                    strcpy(options->loadFile, optarg);
                    break;
                case 'f':
                    sscanf(optarg, "%u", &(options->numFilters));
                    break;
                case 't':
                    options->testFile = malloc(strlen(optarg) + 1);
                    strcpy(options->testFile, optarg);
                    break;
                case 'x':
                    sscanf(optarg, "%u", &(options->trieDepth));
                    break;
                case 's':
                    sscanf(optarg, "%u", &(options->numFilters));
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
                        FIBCaesar *caesarFIB = fibCaesar_Create(options->filterSize, options->filterSize, options->numFilters);
                        fib = fib_Create(caesarFIB, CaesarFIBAsFIB);
                    } else if (strcmp(optarg, "caesar-filter") == 0) {
                        FIBCaesarFilter *filterFIB = fibCaesarFilter_Create(options->numPorts, options->filterSize, options->filterSize, options->numFilters);
                        fib = fib_Create(filterFIB, CaesarFilterFIBAsFIB);
                    } else if (strcmp(optarg, "merged-filter") == 0) {
                        FIBMergedFilter *filterFIB = fibMergedFilter_Create(options->numPorts, options->filterSize, options->numFilters);
                        fib = fib_Create(filterFIB, MergedFilterFIBAsFIB);
                    } else if (strcmp(optarg, "patricia") == 0) {
                        FIBPatricia *patriciaFIB = fibPatricia_Create();
                        fib = fib_Create(patriciaFIB, PatriciaFIBAsFIB);
                    } else if (strcmp(optarg, "tbf") == 0) {
                        FIBTBF *tbf = fibTBF_Create(options->trieDepth, options->filterSize, options->numFilters);
                        fib = fib_Create(tbf, TBFAsFIB);
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
                    options->hashSize = atoi(optarg);
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

static Name *
_readNextNameFromFile(FILE *file)
{
    PARCBufferComposer *composer = readLine(file);
    PARCBuffer *bufferString = parcBufferComposer_ProduceBuffer(composer);
    parcBufferComposer_Release(&composer);
    if (parcBuffer_Remaining(bufferString) == 0) {
        return NULL;
    }

    // Create the original name and store it for later
    char *string = parcBuffer_ToString(bufferString);
    Name *name = name_CreateFromCString(string);

    parcMemory_Deallocate(&string);
    parcBuffer_Release(&bufferString);

    return name;
}

static TimedResultSet *
_loadFIB(FIBOptions *options)
{
    TimedResultSet *timeResults = (TimedResultSet *) malloc(sizeof(TimedResultSet));
    assertTrue(timeResults != NULL, "Failed to allocate a result set. Fail immediately");
    timeResults->head = NULL;
    timeResults->count = 0;
    timeResults->total = 0;

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
        Name *name = _readNextNameFromFile(file);
        if (name == NULL) {
            break;
        }

        if (options->hasher != NULL) {
            Name *newName = name_Hash(name, options->hasher, options->hashSize);
            name_Destroy(&name);
            name = newName;
        }

        if (name_GetSegmentCount(name) > 0) {
            Bitmap *vector = bitmap_Create(options->numPorts);
            assertNotNull(vector, "Could not allocate a PARCBitVector");
            bitmap_Set(vector, num);
            num = (num + 1) % options->numPorts;

            // Insert into the FIB
            struct timespec start = timerStart();
            fib_Insert(fib, name, vector);
            long elapsedTime = timerEnd(start);

            // Record the insertion time
            _appendTimedResult(timeResults, elapsedTime);

            index++;
        }
    } while (true);

    return timeResults;
}

static TimedResultSet *
_testFIB(FIBOptions *options)
{
    TimedResultSet *timeResults = (TimedResultSet *) malloc(sizeof(TimedResultSet));
    assertTrue(timeResults != NULL, "Failed to allocate a result set. Fail immediately");
    timeResults->head = NULL;
    timeResults->count = 0;
    timeResults->total = 0;

    FILE *file = fopen(options->testFile, "r");
    if (file == NULL) {
        perror("Could not open test file");
        usage();
        exit(EXIT_FAILURE);
    }

    // This is the FIB to use
    FIB *fib = options->fib;

    int numFalsePositives = 0;

    int index = 0;
    do {
        Name *name = _readNextNameFromFile(file);
        if (name == NULL) {
            break;
        }

        if (options->hasher != NULL) {
            Name *newName = name_Hash(name, options->hasher, options->hashSize);
            name_Destroy(&name);
            name = newName;
        }

        if (name_GetSegmentCount(name) > 0) {
            // Look up the FIB and time it.
            struct timespec start = timerStart();
            Bitmap *output = fib_LPM(fib, name);
            long elapsedTime = timerEnd(start);

            if (output == NULL) {
                numFalsePositives++;
            }

            // Record the insertion time
            _appendTimedResult(timeResults, elapsedTime);

            index++;
        }
    } while (true);

    _addCount(timeResults, numFalsePositives);

    return timeResults;
}

int
main(int argc, char **argv)
{
    FIBOptions *options = parseCommandLineOptions(argc, argv);

    // Run the test
    TimedResultSet *insertionResults = _loadFIB(options);
    TimedResultSet *testResults = _testFIB(options);

    PARCBasicStats *insertStats = parcBasicStats_Create();
    TimedResult *curr = insertionResults->head;
    while (curr != NULL) {
        parcBasicStats_Update(insertStats, (double) curr->time);
        curr = curr->next;
    }

    double mean = parcBasicStats_Mean(insertStats);
    double stdev = parcBasicStats_StandardDeviation(insertStats);
    double fraction = (double) insertionResults->count / insertionResults->total;
    printf("insert,%2d,%f,%f,%f\n", options->hashSize, mean, stdev, fraction);

    // Display the results
    PARCBasicStats *testStats = parcBasicStats_Create();
    curr = testResults->head;
    while (curr != NULL) {
        parcBasicStats_Update(testStats, (double) curr->time);
        curr = curr->next;
    }

    mean = parcBasicStats_Mean(testStats);
    stdev = parcBasicStats_StandardDeviation(testStats);
    fraction = (double) testResults->count / testResults->total;
    printf("lookup,%2d,%f,%f,%f\n", options->hashSize, mean, stdev, fraction);

    parcBasicStats_Release(&insertStats);
    parcBasicStats_Release(&testStats);

    return EXIT_SUCCESS;
}
