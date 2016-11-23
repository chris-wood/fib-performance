//
// Created by caw on 11/23/16.
//

#include "name.h"

#include <parc/algol/parc_Buffer.h>
#include <parc/algol/parc_Memory.h>

#include <ccnx/common/ccnx_Name.h>
#include <ccnx/common/codec/ccnxCodec_TlvEncoder.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_NameCodec.h>

struct name {
    char *uri;
    PARCBuffer *wireFormat;

    int numSegments;
    int *offsets;

};

static PARCBuffer *
_encodeNameToWireFormat(Name *name)
{
    CCNxName *ccnxName = ccnxName_CreateFromCString(name->uri);
    if (ccnxName != NULL) {

        name->numSegments = ccnxName_GetSegmentCount(ccnxName);
        name->offsets = parcMemory_Allocate(sizeof(int) * name->numSegments);

        CCNxCodecTlvEncoder *codec = ccnxCodecTlvEncoder_Create();
        ccnxCodecTlvEncoder_Initialize(codec);

        size_t numBytes = ccnxCodecSchemaV1NameCodec_Encode(codec, 0, ccnxName);

        ccnxCodecTlvEncoder_Finalize(codec);
        PARCBuffer *buffer = ccnxCodecTlvEncoder_CreateBuffer(codec);

        ccnxCodecTlvEncoder_Destroy(&codec);

        ccnxName_Release(&ccnxName);
        return buffer;
    }
    ccnxName_Release(&ccnxName);
    return NULL;
}

static void
_createSegmentIndex(Name *name)
{
    int offset = 4; // skip past the name TL 4 byte container
    int totalSize = parcBuffer_Remaining(name->wireFormat);
    int segmentIndex = 0;

    while (offset < totalSize) {
        name->offsets[segmentIndex++] = offset;

        uint8_t msb = parcBuffer_GetAtIndex(name->wireFormat, offset + 2);
        uint8_t lsb = parcBuffer_GetAtIndex(name->wireFormat, offset + 3);
        uint16_t segmentSize = (((uint16_t) msb) << 8) | ((uint16_t) lsb);
        offset += 4 + segmentSize;
    }
}

Name *
name_CreateFromCString(char *uri)
{
    Name *name = parcMemory_Allocate(sizeof(Name));
    if (name != NULL) {
        name->uri = parcMemory_StringDuplicate(uri, strlen(uri));
        name->wireFormat = _encodeNameToWireFormat(name);
        _createSegmentIndex(name);
    }
    return name;
}

void
name_Destroy(Name **nameP)
{
    Name *name = *nameP;
    parcBuffer_Release(&name->wireFormat);
    parcMemory_Deallocate(&name->offsets);

    parcMemory_Deallocate(nameP);
    *nameP = NULL;
}