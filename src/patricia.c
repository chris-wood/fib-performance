#include <stdlib.h>
#include <stdio.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>

#include "patricia.h"

struct _patricia_node;
typedef struct _patricia_node {
    PARCBuffer *label;
    void *value;
    bool isLeaf;

    int numEdges;
    struct _patricia_node **edges;
} _PatriciaNode;

void
_patriciaNode_Destroy(_PatriciaNode **nodeP)
{
    _PatriciaNode *node = *nodeP;

    if (node->label != NULL) {
        parcBuffer_Release(&node->label);
    }
    for (int i = 0; i < node->numEdges; i++) {
        _patriciaNode_Destroy(&node->edges[i]);
    }

    free(node);
    *nodeP = NULL;
}

_PatriciaNode *
_patriciaNode_CreateLeaf(PARCBuffer *label, void *value)
{
    _PatriciaNode *node = (_PatriciaNode *) malloc(sizeof(_PatriciaNode));
    if (node != NULL) {
        node->label = parcBuffer_Acquire(label);
        node->isLeaf = true;
        node->numEdges = 0;
        node->edges = NULL;
        node->value = value;
    }
    return node;
}

void
_patriciaNode_AddEdge(_PatriciaNode *node, _PatriciaNode *edge) 
{
    node->numEdges++;
    if (node->edges == NULL) { 
        node->edges = (_PatriciaNode **) malloc(sizeof(_PatriciaNode *) * node->numEdges);
    } else {
        node->edges = (_PatriciaNode **) realloc(node->edges, sizeof(_PatriciaNode *) * node->numEdges);
    }
    node->edges[node->numEdges - 1] = edge;
    node->isLeaf = false;
}

static void
_patriciaNode_Display(_PatriciaNode *node, int indentation)
{
    printf("|");
    for (int i = 0; i < indentation; i++) {
        printf("-");
    }
    char *label = parcBuffer_ToString(node->label);
    printf("> %s\n", label);
    parcMemory_Deallocate(&label);

    for (int i = 0; i < node->numEdges; i++) {
        _patriciaNode_Display(node->edges[i], indentation + 2);
    };
}

struct patricia {
    _PatriciaNode *head;
};

Patricia *
patricia_Create()
{
    Patricia *patricia = (Patricia *) malloc(sizeof(Patricia));
    if (patricia != NULL) {
        PARCBuffer *emptyLabel = parcBuffer_Allocate(1);
        patricia->head = _patriciaNode_CreateLeaf(emptyLabel, NULL);
        parcBuffer_Release(&emptyLabel);
    }
    return patricia;
}

void 
patricia_Destroy(Patricia **patriciaP)
{
    Patricia *patricia = *patriciaP;
    _patriciaNode_Destroy(&patricia->head);
    free(patricia);
    *patriciaP = NULL;
}

static void
_patricia_Display(Patricia *trie)
{
    _patriciaNode_Display(trie->head, 0);
}

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static int
_sharedPrefix(PARCBuffer *x, PARCBuffer *y) 
{
    int minComponents = MIN(parcBuffer_Remaining(x), parcBuffer_Remaining(y));   

    int shared = 0;
    for (size_t i = 0; i < minComponents; i++) {
        if (parcBuffer_GetAtIndex(x, parcBuffer_Position(x) + i) != parcBuffer_GetAtIndex(y, parcBuffer_Position(y) + i)) {
            return shared;
        }
        shared++;
    }

    return shared;
}

void 
patricia_Insert(Patricia *trie, PARCBuffer *key, void *item)
{
    _PatriciaNode *current = trie->head;
    if (current->isLeaf) {
        current = NULL;
    }

    int elementsFound = 0;
    size_t labelLength = parcBuffer_Remaining(key);
    _PatriciaNode *prev = NULL;

    // Search until we can make no further progress.
    PARCBuffer *prefix = parcBuffer_Copy(key);
    while (current != NULL && !current->isLeaf && elementsFound < labelLength) {
        _PatriciaNode *curr = current;
        current = NULL;
        for (int i = 0; i < curr->numEdges; i++) {
            _PatriciaNode *next = curr->edges[i];

            int prefixCount = parcBuffer_Remaining(prefix);
            int sharedCount = _sharedPrefix(prefix, next->label);

            if (prefixCount == sharedCount) { // jump to the next level
                prev = curr;
                current = next;

                size_t nextLabelLength = parcBuffer_Remaining(next->label);
                elementsFound += nextLabelLength;

                parcBuffer_SetPosition(prefix , parcBuffer_Position(prefix) + nextLabelLength);
                break;
            } else if (sharedCount > 0) { // split up the node and return
                // abc | abd -> ab, c, d
                PARCBuffer *sharedPrefix = parcBuffer_Copy(next->label); // "ab"
                PARCBuffer *leftPrefix = parcBuffer_Copy(sharedPrefix); // "c"
                PARCBuffer *rightPrefix = parcBuffer_Copy(prefix); // "d"

                parcBuffer_SetLimit(sharedPrefix, sharedCount);
                parcBuffer_SetPosition(leftPrefix, parcBuffer_Position(leftPrefix) + sharedCount);
                parcBuffer_SetPosition(rightPrefix, parcBuffer_Position(rightPrefix) + sharedCount);

                _PatriciaNode *splitRight = _patriciaNode_CreateLeaf(rightPrefix, item);
                _PatriciaNode *splitLeft = _patriciaNode_CreateLeaf(sharedPrefix, NULL); // no item goes here
                _PatriciaNode *newLeft = _patriciaNode_CreateLeaf(leftPrefix, next->value); // no item goes here

                curr->edges[i] = splitLeft;
                _patriciaNode_Destroy(&next);

                _patriciaNode_AddEdge(splitLeft, newLeft);
                _patriciaNode_AddEdge(splitLeft, splitRight);

                parcBuffer_SetPosition(prefix, parcBuffer_Position(prefix) + parcBuffer_Remaining(prefix));

                parcBuffer_Release(&sharedPrefix);
                parcBuffer_Release(&leftPrefix);
                parcBuffer_Release(&rightPrefix);

                parcBuffer_Release(&prefix);
                return;
            } else { // nothing in common
                current = NULL; // does nothing.
            }
        }
    }
 
    // Handle the insertion into the trie
    _PatriciaNode *newEdge = _patriciaNode_CreateLeaf(prefix, item);
    if (current == NULL) {
        _PatriciaNode *target = prev == NULL ? trie->head : prev;
        _patriciaNode_AddEdge(target, newEdge);
    } else if (current->isLeaf) {
        _patriciaNode_AddEdge(current, newEdge);
    }

    parcBuffer_Release(&prefix);
}

void *
patricia_Get(Patricia *trie, PARCBuffer *key)
{
    int elementsFound = 0;
    size_t labelLength = parcBuffer_Remaining(key);
    _PatriciaNode *current = trie->head;

    PARCBuffer *prefix = parcBuffer_Slice(key);
    while (current != NULL && !current->isLeaf && elementsFound < labelLength) {
        _PatriciaNode *curr = current;
        current = NULL;
        for (int i = 0; i < curr->numEdges; i++) {
            _PatriciaNode *next = curr->edges[i];
            
            int sharedCount = _sharedPrefix(prefix, next->label);
            if (sharedCount > 0) { // jump to the next level
                current = next;
                size_t nextLabelLength = parcBuffer_Remaining(next->label);
                if (sharedCount < nextLabelLength) {
                    parcBuffer_Release(&prefix);
                    return NULL;
                }
                elementsFound += sharedCount;
                parcBuffer_SetPosition(prefix, parcBuffer_Position(prefix) + sharedCount);
                break;
            } 
        }
    }

    parcBuffer_Release(&prefix);
    
    if (current == NULL) {
        return NULL;
    } else if (current->isLeaf && elementsFound <= labelLength) {
        return current->value;
    } else {
        return NULL;
    }
}


