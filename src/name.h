//
// Created by caw on 11/23/16.
//

#ifndef FIB_PERF_NAME_H
#define FIB_PERF_NAME_H

struct name;
typedef struct name Name;

Name *name_CreateFromCString(char *uri);
void name_Destroy(Name **nameP);

#endif //FIB_PERF_NAME_H
