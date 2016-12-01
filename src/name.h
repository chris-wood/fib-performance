//
// Created by caw on 11/23/16.
//

#ifndef FIB_PERF_NAME_H
#define FIB_PERF_NAME_H

#include <parc/algol/parc_Buffer.h>

struct name;
typedef struct name Name;

Name *name_CreateFromCString(char *uri);
void name_Destroy(Name **nameP);

void name_Display(const Name *name);
int name_GetSegmentCount(const Name *name);
PARCBuffer *name_GetWireFormat(const Name *name, int n);
int name_GetSegmentLength(const Name *name, int n);
uint8_t *name_GetSegmentOffset(const Name *name, int n);

#endif //FIB_PERF_NAME_H
