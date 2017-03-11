#include "name_reader.h"

#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_BufferComposer.h>
#include <parc/statistics/parc_BasicStats.h>

#include <stdio.h>
#include <ctype.h>
#include <getopt.h>

typedef struct name_list {
    Name *name;
    struct name_list *next;
} NameNode;

struct name_reader {
    char *fname;
    NameNode *head;
    NameNode *current;
};

NameNode *
nameNode_Create(Name *name)
{
    NameNode *node = (NameNode *) malloc(sizeof(NameNode));
    if (node != NULL) {
        node->name = name;
        node->next = NULL;
    }
    return node;
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

void
nameReader_Destroy(NameReader **readerP)
{
    NameReader *reader = *readerP;

    if (reader->fname != NULL) {
        free(reader->fname);
    }

    // XXX: delete the NameNode list
}

NameReader *
nameReader_CreateFromFile(char *fileName, Hasher *hasher)
{
    NameReader *reader = (NameReader *) malloc(sizeof(NameReader));
    if (reader == NULL) {
        return NULL;
    }

    reader->fname = malloc(strlen(fileName));
    strncpy(reader->fname, fileName, strlen(fileName));

    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        fprintf(stderr, "Could not open name file: %s\n", fileName);
        nameReader_Destroy(reader);
        return NULL;
    }

    NameNode **head = &reader->head;

    do {
        Name *name = _readNextNameFromFile(file);
        if (name == NULL) {
            break;
        }

//        if (hasher != NULL) {
//            Name *newName = name_Hash(name, options->hasher, options->hashSize);
//            name_Destroy(&name);
//            name = newName;
//        }

        if (name_GetSegmentCount(name) > 0) {
            NameNode *node = nameNode_Create(name);
            *head = node;
            head = &node->next;
        }
    } while (true);

//    NameNode *node = reader->head;
//    while (node != NULL) {
//        printf("Read: %s\n", name_GetNameString(node->name));
//        node = node->next;
//    }

    reader->current = reader->head;
    return reader;
}

bool
nameReader_HasNext(NameReader *reader)
{
    return reader->current != NULL;
}

Name *
nameReader_Next(NameReader *reader)
{
    Name *name = reader->current->name;
    reader->current = reader->current->next;
    return name;
}