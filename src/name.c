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

static void
_encodeNameToWireFormat(Name *name)
{
    CCNxName *ccnxName = ccnxName_CreateFromCString(name->uri);
    if (ccnxName != NULL) {
        name->numSegments = ccnxName_GetSegmentCount(ccnxName);
        name->offsets = parcMemory_Allocate(sizeof(int) * name->numSegments);
        name->sizes = parcMemory_Allocate(sizeof(int) * name->numSegments);
        name->isHashed = false;

        CCNxCodecTlvEncoder *codec = ccnxCodecTlvEncoder_Create();
        ccnxCodecTlvEncoder_Initialize(codec);

        ssize_t numBytes = ccnxCodecSchemaV1NameCodec_Encode(codec, 0, ccnxName);
        assertTrue(numBytes > 0, "Failed to encode the name: %lu", numBytes);

        ccnxCodecTlvEncoder_Finalize(codec);
        PARCBuffer *buffer = ccnxCodecTlvEncoder_CreateBuffer(codec);
        parcBuffer_SetPosition(buffer, parcBuffer_Position(buffer) + 4); // skip past the TL container header

        ccnxCodecTlvEncoder_Destroy(&codec);

        name->wireFormat = parcBuffer_Acquire(buffer);
        parcBuffer_Release(&buffer);
        ccnxName_Release(&ccnxName);
    }
}

static void
_createSegmentIndex(Name *name)
{
    int offset = 0;
    int totalSize = parcBuffer_Remaining(name->wireFormat);
    int segmentIndex = 0;

    size_t position = parcBuffer_Position(name->wireFormat);

    while (offset < totalSize) {
        name->offsets[segmentIndex] = offset;

        // Swallow the type
        parcBuffer_GetUint16(name->wireFormat);

        // Extract the size
        uint16_t size = parcBuffer_GetUint16(name->wireFormat);

        name->sizes[segmentIndex++] = size;

        parcBuffer_SetPosition(name->wireFormat, parcBuffer_Position(name->wireFormat) + size);
        offset += 4 + size;
    }

    parcBuffer_SetPosition(name->wireFormat, position);
}

Name *
name_CreateFromCString(char *uri)
{
    Name *name = parcMemory_Allocate(sizeof(Name));
    if (name != NULL) {
        name->uri = parcMemory_StringDuplicate(uri, strlen(uri));
        _encodeNameToWireFormat(name);
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

void
name_Display(const Name *name)
{
    printf("%s\n", name->uri);
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
