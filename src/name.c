//
// Created by caw on 11/23/16.
//

#include "name.h"

#include <stdio.h>

#include <parc/algol/parc_Memory.h>

#include <ccnx/common/ccnx_Name.h>
#include <ccnx/common/codec/ccnxCodec_TlvEncoder.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_NameCodec.h>

struct name {
    char *uri;
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

    parcMemory_Deallocate(&name->uri);
    parcBuffer_Release(&name->wireFormat);
    parcMemory_Deallocate(&name->offsets);
    parcMemory_Deallocate(&name->sizes);

    parcMemory_Deallocate(nameP);
    *nameP = NULL;
}

void
name_Display(const Name *name)
{
    printf("%s\n", name->uri);
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

    PARCByteArray *byteArray = parcByteArray_Wrap(capacity, parcBuffer_Overlay(name->wireFormat, 0));
    PARCBuffer *buffer = parcBuffer_WrapByteArray(byteArray, 0, capacity);
    parcByteArray_Release(&byteArray);

    return buffer;
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