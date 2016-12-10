//
// Created by caw on 11/23/16.
//

#ifndef FIB_PERF_NAME_H
#define FIB_PERF_NAME_H

#include <parc/algol/parc_Buffer.h>

#include "hasher.h"

struct name;
typedef struct name Name;

Name *name_CreateFromCString(char *uri);
void name_Destroy(Name **nameP);

Name *name_Hash(Name *name, Hasher *hasher, int hashSize);
bool name_IsHashed(const Name *name);

void name_Display(const Name *name);
char *name_GetNameString(const Name *name);
int name_GetSegmentCount(const Name *name);
PARCBuffer *name_GetWireFormat(const Name *name, int n);
PARCBuffer *name_GetSegmentWireFormat(const Name *name, int n);
int name_GetSegmentLength(const Name *name, int n);
uint8_t *name_GetSegmentOffset(const Name *name, int n);

PARCBuffer *name_XORSegment(const Name *name, int index, PARCBuffer *vector);

#endif // FIB_PERF_NAME_H
