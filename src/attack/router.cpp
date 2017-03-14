//
// Created by Christopher Wood on 3/5/17.
//

#include "router.h"
#include "../timer.h"
#include <iostream>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

using namespace std;

void
Router::ConnectSink(int sock)
{
    sinkfd = sock;
}

void
Router::ConnectSource(int sock)
{
    sourcefd = sock;
}

#define MAX_NAME_SIZE 64000

int
Router::LoadNames(NameReader *reader)
{
    return LoadHashedNames(reader, NULL);
}

int
Router::LoadTestNames(NameReader *reader)
{
    return LoadHashedTestNames(reader, NULL);
}

int
Router::LoadHashedNames(NameReader *reader, Hasher *hasher)
{
    int index = 0;
    int capacity = 10;
    while (nameReader_HasNext(reader)) {
        Name *name = nameReader_Next(reader);

        if (hasher != NULL) {
            Name *newName = name_Hash(name, hasher, 32);
            name_Destroy(&name);
            name = newName;
        }

        Bitmap *vector = bitmap_Create(32);
        bitmap_Set(vector, index % capacity);
        fib_Insert(fib, name, vector);
        index++;
    }
    return index;
}

int
Router::LoadHashedTestNames(NameReader *reader, Hasher *hasher)
{
    while (nameReader_HasNext(reader)) {
        Name *name = nameReader_Next(reader);

        if (hasher != NULL) {
            Name *newName = name_Hash(name, hasher, 32);
            name_Destroy(&name);
            name = newName;
        }

        names.push_back(name);
    }
    return names.size();
}

void
Router::Run()
{
    uint8_t nameBuffer[MAX_NAME_SIZE];

    for (std::vector<Name *>::iterator itr = names.begin(); itr != names.end(); itr++) {
        Name *name = *itr;

        // Serialize and send the name
        PARCBuffer *nameWireFormat = name_GetWireFormat(name, name_GetSegmentCount(name));
        uint8_t *nameBuffer = (uint8_t *) parcBuffer_Overlay(nameWireFormat, 0);
        size_t nameLength = parcBuffer_Remaining(nameWireFormat);

        // Reconstruct the name
        Name *constructedName = name_CreateFromBuffer(nameWireFormat);

        // Index the name into the FIB -- don't do anything with it though.
        // We're just estimating the time it takes to perform this operation
        Bitmap *output = fib_LPM(fib, name);
        name_Destroy(&name);
        // XXX: assert the outut is not NULL
        // XXX: use the output bitmap to send to the right socket(s)

        // Record the time it was done being processed
        struct timespec now = timerStart();
        outTimes.push_back(now);
    }
}

void
Router::ProcessInputs()
{
    for (std::vector<Name *>::iterator itr = names.begin(); itr != names.end(); itr++) {
        Name *name = *itr;

        // Record the time it started being processed
        struct timespec start = timerStart();
        inTimes.push_back(start);
    }
}

void *
processInputs(void *arg)
{
    Router *router = (Router *) arg;
    router->ProcessInputs();
    return NULL;
}

void *
runRouter(void *arg)
{
    Router *router = (Router *) arg;
    router->Run();
    return NULL;
}