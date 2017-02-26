#include <stdlib.h>
#include <stdio.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>

#include "patricia.h"

typedef struct {
    void *value;
    int refCount;
    void (*valueDestructor)(void **valueP);
} _PatriciaNodeValue;

static _PatriciaNodeValue *
_patriciaNodeValue_Create(void *item, void (*valueDestructor)(void **valueP))
{
    _PatriciaNodeValue *value = (_PatriciaNodeValue *) malloc(sizeof(_PatriciaNodeValue));
    if (value != NULL) {
        value->value = item;
        value->refCount = 1;
        value->valueDestructor = valueDestructor;
    }
    return value;
}

static void *
_patriciaNodeValue_Value(_PatriciaNodeValue *value)
{
    if (value != NULL) {
        return value->value;
    }
    return NULL;
}

static _PatriciaNodeValue *
_patriciaNodeValue_Acquire(_PatriciaNodeValue *value)
{
    if (value == NULL) {
        return NULL;
    }

    value->refCount++;

    return value;
}

static void
_patriciaNodeValue_Release(_PatriciaNodeValue **valueP)
{
    _PatriciaNodeValue *value = (_PatriciaNodeValue *) *valueP;
    if (value != NULL) {
        return;
    }

    value->refCount--;
    if (value->refCount == 0) {
        if (value->valueDestructor != NULL && value->value != NULL) {
            value->valueDestructor(&(value->value));
        }
        free(value);
        *valueP = NULL;
    }
}

struct _patricia_node;
typedef struct _patricia_node {
    PARCBuffer *label;
    _PatriciaNodeValue *value;

    bool isLeaf;

    int numChildren;
    struct _patricia_node **children;
} _PatriciaNode;

void
_patriciaNode_Destroy(_PatriciaNode **nodeP)
{
    _PatriciaNode *node = *nodeP;

    if (node->label != NULL) {
        parcBuffer_Release(&node->label);
    }
    for (int i = 0; i < node->numChildren; i++) {
        _patriciaNode_Destroy(&node->children[i]);
    }
    if (node->value != NULL) {
        _patriciaNodeValue_Release(&(node->value));
    }

    free(node);
    *nodeP = NULL;
}

_PatriciaNode *
_patriciaNode_CreateLeaf(PARCBuffer *label, _PatriciaNodeValue *value)
{
    _PatriciaNode *node = (_PatriciaNode *) malloc(sizeof(_PatriciaNode));
    if (node != NULL) {
        node->label = parcBuffer_Acquire(label);
        node->isLeaf = true;
        node->numChildren = 0;
        node->children = NULL;
        node->value = _patriciaNodeValue_Acquire(value);
    }
    return node;
}

void
_patriciaNode_AddEdge(_PatriciaNode *node, _PatriciaNode *edge) 
{
    node->numChildren++;
    if (node->children == NULL) {
        node->children = (_PatriciaNode **) malloc(sizeof(_PatriciaNode *) * node->numChildren);
    } else {
        node->children = (_PatriciaNode **) realloc(node->children, sizeof(_PatriciaNode *) * node->numChildren);
    }
    node->children[node->numChildren - 1] = edge;
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

    for (int i = 0; i < node->numChildren; i++) {
        _patriciaNode_Display(node->children[i], indentation + 2);
    };
}

struct patricia {
    _PatriciaNode *head;
    void (*valueDestructor)(void **valueP);
};

Patricia *
patricia_Create(void (*valueDestructor)(void **valueP))
{
    Patricia *patricia = (Patricia *) malloc(sizeof(Patricia));
    if (patricia != NULL) {
        PARCBuffer *emptyLabel = parcBuffer_Allocate(1);
        patricia->head = _patriciaNode_CreateLeaf(emptyLabel, NULL);
        patricia->valueDestructor = valueDestructor;
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

void
patricia_Display(Patricia *trie)
{
    _patriciaNode_Display(trie->head, 0);
}

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static int
_sharedPrefix(PARCBuffer *x, PARCBuffer *y) 
{
    assertTrue(x != NULL && y != NULL, "Invalid state: one of the buffers is NULL");

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
patricia_Insert(Patricia *trie, PARCBuffer *key, void *opaqueValue)
{
    _PatriciaNodeValue *value = _patriciaNodeValue_Create(opaqueValue, trie->valueDestructor);

    _PatriciaNode *current = trie->head;
    if (current->isLeaf) {
        current = NULL;
    }

    PARCBuffer *prefix = parcBuffer_Duplicate(key);
    _PatriciaNode *newEdge = _patriciaNode_CreateLeaf(prefix, value);

    // Base case, when we've got nothing in the trie
    if (current == NULL) {
        _patriciaNode_AddEdge(trie->head, newEdge);
        parcBuffer_Release(&prefix);
        return;
    }

    // Otherwise, search for the insertion spot
    size_t elementsFound = 0;
    size_t prefixLength = parcBuffer_Remaining(prefix);
    _PatriciaNode *prev = NULL;

    while (elementsFound < prefixLength) {
        // If we've reached a leaf and haven't added all of our symbols, then just tack it on to the current node
        if (current->isLeaf) {
            _patriciaNode_AddEdge(current, newEdge);
            parcBuffer_Release(&prefix);
            return;
        }

        // Otherwise, traverse down the node's children
        bool inserted = false;
        for (int i = 0; i < current->numChildren; i++) {
            prev = current;
            _PatriciaNode *next = current->children[i];
            int prefixCount = parcBuffer_Remaining(next->label);
            int sharedCount = _sharedPrefix(prefix, next->label);

            // If we consumed all of the current node, go to the next layer
            if (sharedCount == prefixCount ) {
                parcBuffer_SetPosition(prefix, parcBuffer_Position(prefix) + sharedCount);
                elementsFound += sharedCount;
                current = next;
                inserted = true;
                break;
            } else if (sharedCount > 0) {
                PARCBuffer *sharedPrefix = parcBuffer_Duplicate(next->label); // "ab"
                PARCBuffer *leftPrefix = parcBuffer_Duplicate(sharedPrefix); // "c"
                PARCBuffer *rightPrefix = parcBuffer_Duplicate(prefix); // "d"

                parcBuffer_SetLimit(sharedPrefix, parcBuffer_Position(sharedPrefix) + sharedCount);
                parcBuffer_SetPosition(leftPrefix, parcBuffer_Position(leftPrefix) + sharedCount);
                parcBuffer_SetPosition(rightPrefix, parcBuffer_Position(rightPrefix) + sharedCount);

                _PatriciaNode *newNode = _patriciaNode_CreateLeaf(sharedPrefix, NULL); // this becomes a non-leaf node
                _PatriciaNode *splitRight = _patriciaNode_CreateLeaf(rightPrefix, value);
                _PatriciaNode *splitLeft = _patriciaNode_CreateLeaf(leftPrefix, next->value);

                current->children[i] = newNode;
                _patriciaNode_AddEdge(newNode, splitLeft);
                _patriciaNode_AddEdge(newNode, splitRight);

                _patriciaNode_Destroy(&next);

                parcBuffer_Release(&sharedPrefix);
                parcBuffer_Release(&leftPrefix);
                parcBuffer_Release(&rightPrefix);
                _patriciaNode_Destroy(&newEdge);
                parcBuffer_Release(&prefix);
                return;
            }
        }

        if (!inserted) { // there's nothing in common with any child, so add to the previous edge
            _patriciaNode_AddEdge(prev, newEdge);
            parcBuffer_Release(&prefix);
            return;
        }
    }
}

void *
patricia_Get(Patricia *trie, PARCBuffer *key)
{
    int elementsFound = 0;
    size_t labelLength = parcBuffer_Remaining(key);
    _PatriciaNode *current = trie->head;
    _PatriciaNode *prev = NULL;

    PARCBuffer *prefix = parcBuffer_Slice(key);
    while (current != NULL && !current->isLeaf && elementsFound < labelLength) {
        _PatriciaNode *curr = current;
        current = NULL;
        for (int i = 0; i < curr->numChildren; i++) {
            _PatriciaNode *next = curr->children[i];

            int prefixCount = parcBuffer_Remaining(next->label);
            int sharedCount = _sharedPrefix(prefix, next->label);

            // If we consumed all of the current node, go to the next layer
            if (sharedCount == prefixCount ) {
                parcBuffer_SetPosition(prefix, parcBuffer_Position(prefix) + sharedCount);
                elementsFound += sharedCount;
                prev = curr;
                current = next;
                break;
            }
//            else if (sharedCount > 0) { // jump to the next level
//                current = next;
//                size_t nextLabelLength = parcBuffer_Remaining(next->label);
//                if (sharedCount < nextLabelLength) {
//                    parcBuffer_Release(&prefix);
//                    return NULL;
//                }
//                elementsFound += sharedCount;
//                parcBuffer_SetPosition(prefix, parcBuffer_Position(prefix) + sharedCount);
//                prev = curr;
//                break;
//            }
        }
    }

    parcBuffer_Release(&prefix);
    
    if (current == NULL && prev == NULL) {
        return prev;
    } else if (current == NULL) {
        return _patriciaNodeValue_Value(prev->value);
    } else if (current->isLeaf && elementsFound <= labelLength) {
        return _patriciaNodeValue_Value(current->value);
    } else {
        return NULL;
    }
}


