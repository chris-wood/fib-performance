//
// Created by Christopher Wood on 3/5/17.
//

#include "router.h"
#include <iostream>
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

#define MAX_NAME_SIZE sizeof(uint16_t)

void
Router::LoadNames(NameReader *reader)
{
    int index = 0;
    int capacity = 10;
    while (nameReader_HasNext(reader)) {
        Name *name = nameReader_Next(reader);
        printf("Inserting: %s\n", name_GetNameString(name));
        Bitmap *vector = bitmap_Create(32);
        bitmap_Set(vector, index++);
        fib_Insert(fib, name, vector);
        index &= capacity - 1;
    }
}

void
Router::Run()
{
    uint8_t nameBuffer[MAX_NAME_SIZE];
    for (int i = 0; i < numberOfNames; i++) {
        std::cout << "router processing name " << i << std::endl;

        // Peek at the length of the name TLV
        if (read(sourcefd, nameBuffer, 2) < 0) {
            std::cerr << "failed to read the header of name " << i << " from the socket" << std::endl;
            return;
        }

        // Read the rest of the name
        uint16_t length = (((uint16_t)nameBuffer[0]) << 8) | (uint16_t)nameBuffer[1];
        std::cout << "reading a name of length " << length << std::endl;
        if (read(sourcefd, nameBuffer + 2, length) < 0) {
            std::cerr << "failed to read the contents of name " << i << " from the socket" << std::endl;
            return;
        }

        // Reconstruct the name
        PARCBuffer *wireFormat = parcBuffer_Wrap(nameBuffer + 2, length, 0, length);
        Name *name = name_CreateFromBuffer(wireFormat);

        // Index the name into the FIB -- don't do anything with it though.
        // We're just estimating the time it takes to perform this operation
        Bitmap *output = fib_LPM(fib, name);
        // XXX: assert the outut is not NULL
        // XXX: use the output bitmap to send to the right socket(s)

        // Blindly write the name to the output socket
        write(sinkfd, nameBuffer, 2); // write the size and then the rest of the name
        if (write(sinkfd, nameBuffer + 2, length) < 0) {
            std::cerr << "failed to write name " << i << " to the sink socket" << std::endl;
        }
    }
}

void *
runRouter(void *arg)
{
    Router *router = (Router *) arg;
    router->Run();
    return NULL;
}