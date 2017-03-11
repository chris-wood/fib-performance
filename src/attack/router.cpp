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

#define MAX_NAME_SIZE 1000

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
        if (read(sourcefd, nameBuffer, 4) < 0) {
            std::cerr << "failed to read the header of name " << i << " from the socket" << std::endl;
            return;
        }

        // Read the rest of the name
        uint16_t length = (((uint16_t)nameBuffer[2]) << 8) | (uint16_t)nameBuffer[3];
        int remaining = 0;
        int numread = read(sourcefd, nameBuffer, length);

//        if ( < 0) {
//            std::cerr << "failed to read the body of name " << i << " from the socket" << std::endl;
//            return;
//        }

        // Reconstruct the name
        PARCBuffer *nameBuffer = parcBuffer_Wrap(nameBuffer, length + 4, 0, length + 4);
        Name *name = name_CreateFromBuffer(nameBuffer);

        // Index the name into the FIB
        Bitmap *output = fib_LPM(fib, name);
        // XXX: assert the outut is not NULL
        // XXX: use the output bitmap to send to the right socket(s)

        // Blindly write the name to the output socket
        if (write(sinkfd, nameBuffer, length + 4) < 0) {
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