#include <stdlib.h>

#include "patricia.h"

struct _patricia_node;
typedef struct {
    PARCBuffer *label;;
    bool isLeaf;

    int numEdges;
    struct _patricia_node *edges;
} _PatriciaNode;

_PatriciaNode *
_patriciaNode_CreateLeaf(PARCBuffer *label)
{
    _PatriciaNode *node = (_PatriciaNode *) malloc(sizeof(_PatriciaNode));
    if (node != NULL) {
        node->label = parcBuffer_Acquire(label); // key used to get here..
        node->isLeaf = true;
        node->edges = NULL;
    }
    return node;
}

void
_patriciaNode_AddEdge(uint8_t label) 
{
    // XXX
}

struct patricia {
    _PatriciaNode *head;
};

Patricia *
patricia_Create()
{
    Patricia *patricia = (Patricia *) malloc(sizeof(Patricia));
    if (patricia != NULL) {
        patricia->head = NULL;
    }
    return patricia;
}

void 
patricia_Destroy(Patricia **patriciaP)
{
    Patricia *patricia = *patriciaP;
    free(patricia);
    *patricia = NULL;
}

void 
patricia_Insert(Patricia *trie, PARCBuffer *key, void *item)
{
    if (trie->head == NULL) {
        _PatriciaNode *head = _patriciaNode_CreateLeaf(key);
        trie->head = head;
        return;
    }

    int elementsFound = 0;
    size_t labelLength = parcBuffer_Remaining(key);
    _PatriciaNode *prev = NULL;
    _PatriciaNode *current = trie->head;

    PARCBuffer *prefix = parcBuffer_Copy(key);
    while (current != NULL && !current->isLeaf && elementsFound < labelLength) {
        for (int i = 0; i < current->numEdges; i++) {
            _PatriciaNode *next = current->edges[i];

            if (_isPrefix(prefix, next->label)) {
                prev = current;
                current = next;
                size_t nextLabelLength = parcBuffer_Remaining(next->label);
                elementsFound += nextLabelLength; 
                prefix = parcBuffer_Slice(prefix);
                parcBuffer_SetPosition(prefix, parcBuffer_Position(prefix) + nextLabelLength);
            } else {
                current = NULL;
            }
        }
    }
 
    if (current == NULL && prev == NULL) {
        // XXX
    } else if (current == NULL) {
        // XXX
    } else {
        // XXX
    }
}

static bool
_isPrefix(PARCBuffer *x, PARCBuffer *y) 
{
    if (parcBuffer_Remaining(x) > parcBuffer_Remaining(y)) {
        return false;
    }

    for (size_t i = 0; i < parcBuffer_Remaining(x); i++) {
        if (parcBuffer_GetAtIndex(x, i) != parcBuffer_GetAtIndex(y, i)) {
            return false;
        }
    }

    return true;
}

void *
patricia_Get(Patricia *trie, PARCBuffer *key)
{
    int elementsFound = 0;
    size_t labelLength = parcBuffer_Remaining(key);
    _PatriciaNode *current = trie->head;

    PARCBuffer *prefix = parcBuffer_Copy(key);
    while (current != NULL && !current->isLeaf && elementsFound < labelLength) {
        for (int i = 0; i < current->numEdges; i++) {
            _PatriciaNode *next = current->edges[i];

            if (_isPrefix(prefix, next->label)) {
                current = next;
                size_t nextLabelLength = parcBuffer_Remaining(next->label);
                elementsFound += nextLabelLength; 
                prefix = parcBuffer_Slice(prefix);
                parcBuffer_SetPosition(prefix, parcBuffer_Position(prefix) + nextLabelLength);
            } else {
                current = NULL;
            }
        }
    }
    
    if (current == NULL) {
        return false;
    } else if (current->isLeaf && elementsFound == labelLength) {
        return true;
    } else {
        return false;
    }
}

#endif // patricia_h_
