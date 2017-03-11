//
// Created by caw on 11/23/16.
//

#include "name.h"
#include "siphasher.h"

#include <stdio.h>

#include <parc/algol/parc_Memory.h>

#include <ccnx/common/ccnx_Name.h>
#include <ccnx/common/codec/ccnxCodec_TlvEncoder.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_NameCodec.h>

struct name {
    char *uri;
    bool isHashed;
    PARCBuffer *wireFormat;

    int numSegments;
    int *offsets;
    int *sizes;
};

static PARCBuffer *
_encodeNameToWireFormat(char *uri)
{
    PARCBuffer *buffer = NULL;
    CCNxName *ccnxName = ccnxName_CreateFromCString(uri);

    if (ccnxName_GetSegmentCount(ccnxName) == 0) {
        ccnxName_Release(&ccnxName);
        return NULL;
    }

    if (ccnxName != NULL) {
        CCNxCodecTlvEncoder *codec = ccnxCodecTlvEncoder_Create();
        ccnxCodecTlvEncoder_Initialize(codec);

        ssize_t numBytes = ccnxCodecSchemaV1NameCodec_Encode(codec, 0, ccnxName);
        assertTrue(numBytes > 0, "Failed to encode the name: %lu", numBytes);

        ccnxCodecTlvEncoder_Finalize(codec);
        buffer = ccnxCodecTlvEncoder_CreateBuffer(codec);
        parcBuffer_SetPosition(buffer, parcBuffer_Position(buffer) + 4); // skip past the TL container header

        ccnxName_Release(&ccnxName);
        assertTrue(parcBuffer_IsValid(buffer), "Expected the wire format to be created correctly");
    }
    return buffer;
}

static int
_countSegmentsFromWireFormat(Name *name)
{
    PARCBuffer *buffer = name->wireFormat;

    uint8_t *overlay = parcBuffer_Overlay(buffer, 0);
    int offset = 0;
    int limit = parcBuffer_Remaining(buffer);

    int segments = 0;
    while (offset < limit) {
        uint16_t length = (((uint16_t)overlay[offset + 2]) << 8) | (uint16_t)overlay[offset + 3];
        offset += 4 + length;
        segments++;
    }
    return segments;
}

static void
_createSegmentIndex(Name *name)
{
    int offset = 0;
    int segmentIndex = 0;

    uint8_t *base = parcBuffer_Overlay(name->wireFormat, 0);
    for (int i = 0; i < name->numSegments; i++) {
        // Set the component offset
        name->offsets[segmentIndex] = offset;

        // Extract and set the size of this component
        uint16_t size = (((uint16_t)base[offset + 2]) << 8) | (uint16_t)base[offset + 3];
        name->sizes[segmentIndex++] = size;

        // Advance past this name component
        offset += 4 + size;
    }
}

Name *
name_CreateFromCString(char *uri)
{
    PARCBuffer *buffer = _encodeNameToWireFormat(uri);
    if (buffer != NULL) {
        Name *name = name_CreateFromBuffer(buffer);
        name->uri = parcMemory_StringDuplicate(uri, strlen(uri));
        return name;
    }
    return NULL;
}

Name *
name_CreateFromBuffer(PARCBuffer *buffer)
{
    Name *name = parcMemory_Allocate(sizeof(Name));
    if (name != NULL) {
        name->uri = NULL;
        name->wireFormat = parcBuffer_Acquire(buffer);
        name->numSegments = _countSegmentsFromWireFormat(name);
        name->offsets = parcMemory_Allocate(sizeof(int) * name->numSegments);
        name->sizes = parcMemory_Allocate(sizeof(int) * name->numSegments);
        _createSegmentIndex(name);
    }
    return name;
}

void
name_Destroy(Name **nameP)
{
    Name *name = *nameP;

    if (name->uri != NULL) {
        parcMemory_Deallocate(&name->uri);
    }
    parcBuffer_Release(&name->wireFormat);
    parcMemory_Deallocate(&name->offsets);
    parcMemory_Deallocate(&name->sizes);
    parcMemory_Deallocate(nameP);
    *nameP = NULL;
}

Name *
name_Hash(Name *name, Hasher *hasher, int hashSize)
{
    Name *newName = parcMemory_Allocate(sizeof(Name));
    if (newName!= NULL) {
        newName->uri = NULL;
        newName->numSegments = name->numSegments;
        newName->offsets = parcMemory_Allocate(sizeof(int) * name->numSegments);
        newName->sizes = parcMemory_Allocate(sizeof(int) * name->numSegments);
        newName->isHashed = true;

        newName->wireFormat = parcBuffer_Allocate(name->numSegments * hashSize);
        uint8_t *overlay = NULL;
        if (parcBuffer_Remaining(newName->wireFormat)) {
            overlay = parcBuffer_Overlay(newName->wireFormat, 0);
        }
        for (int i = 1; i <= name->numSegments; i++) {
            PARCBuffer *prefix = name_GetWireFormat(name, i);
            PARCBuffer *hash = hasher_Hash(hasher, prefix);

            memcpy(overlay + (hashSize * (i - 1)), parcBuffer_Overlay(hash, 0), hashSize);
            newName->offsets[i - 1] = i == 1 ? 0 : newName->offsets[i - 2] + newName->sizes[i - 2];
            newName->sizes[i - 1] = hashSize;

            parcBuffer_Release(&hash);
            parcBuffer_Release(&prefix);
        }
    }
    return newName;
}

bool
name_IsHashed(const Name *name)
{
    return name->isHashed;
}

char *
name_GetNameString(const Name *name)
{
    return name->uri;
}

int
name_GetSegmentCount(const Name *name)
{
    return name->numSegments;
}

PARCBuffer *
name_GetWireFormat(const Name *name, int n)
{
    int capacity = n == name->numSegments ? parcBuffer_Remaining(name->wireFormat) : name->offsets[n];
    PARCBuffer *buffer = parcBuffer_Wrap(parcBuffer_Overlay(name->wireFormat, 0), capacity, 0, capacity);
    return buffer;
}

void
name_AssertIsValid(const Name *name)
{
    assertTrue(parcBuffer_IsValid(name->wireFormat), "Invalid wire format");
    assertTrue(name->offsets != NULL, "NULL offsets");
    assertTrue(name->sizes != NULL, "NULL sizes");
}

PARCBuffer *
name_GetSubWireFormat(const Name *name, int start, int end)
{
    assertTrue(parcBuffer_IsValid(name->wireFormat), "Name's wire format is invalid -- what happened?");
    int offset = name->offsets[start];
    int capacity = end == name->numSegments ? parcBuffer_Remaining(name->wireFormat) : name->offsets[end];
    capacity -= offset;

    PARCBuffer *buffer = parcBuffer_Wrap(parcBuffer_Overlay(name->wireFormat, 0), parcBuffer_Remaining(name->wireFormat), offset, parcBuffer_Remaining(name->wireFormat));
    assertTrue(parcBuffer_IsValid(buffer), "Expected buffer to be valid");
    return buffer;
}

PARCBuffer *
name_GetSegmentWireFormat(const Name *name, int n)
{
    return NULL;
}

int
name_GetPrefixLength(const Name *name, int n)
{
    return n == name->numSegments ? parcBuffer_Remaining(name->wireFormat) : name->offsets[n];
}

uint8_t *
name_GetBuffer(const Name *name)
{
    return parcBuffer_Overlay(name->wireFormat, 0);
}

int
name_GetSegmentLength(const Name *name, int n)
{
    return name->sizes[n];
}

uint8_t *
name_GetSegmentOffset(const Name *name, int n)
{
    uint8_t *overlay = parcBuffer_Overlay(name->wireFormat, 0);
    return &(overlay[name->offsets[n]]); // 4 to skip past TL container
}

PARCBuffer *
name_XORSegment(const Name *name, int index, PARCBuffer *vector)
{
    assertTrue(name_IsHashed(name), "Can't call name_XORSegment on a name that wasn't hashed previously");
    int size = name->sizes[0];
    assertTrue(parcBuffer_Remaining(vector) == size, "Size mismatch -- invalid use of name_XORSegment");

    uint8_t *vectorBuffer = parcBuffer_Overlay(vector, 0);
    uint8_t *sourceSegment = name_GetSegmentOffset(name, index);
    PARCBuffer *xor = parcBuffer_Allocate(size); // all segments are of the same length here
    uint8_t *result = parcBuffer_Overlay(xor, 0);
    for (int i = 0; i < size; i++) {
        result[i] = vectorBuffer[i] ^ sourceSegment[i];
    }
    return xor;
}

void
name_Display(const Name *name)
{
    printf("Number of segments: %d\n", name->numSegments);
    printf("Offsets and sizes:\n");
    for (size_t i = 0; i < name->numSegments; i++) {
        printf("\t %d: %d\n", name->offsets[i], name->sizes[i]);
    }
    printf("\n");
}