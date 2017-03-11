#ifdef __cplusplus
extern "C" {
#endif

#ifndef FIB_PERF_NAME_READER_H
#define FIB_PERF_NAME_READER_H

#include "name.h"

struct name_reader;
typedef struct name_reader NameReader;

NameReader *nameReader_CreateFromFile(char *file, Hasher *hasher);

bool nameReader_HasNext(NameReader *reader);

Name *nameReader_Next(NameReader *reader);

#endif //FIB_PERF_NAME_READER_H

#ifdef __cplusplus
}
#endif
