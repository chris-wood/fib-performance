//
// Created by Christopher Wood on 3/5/17.
//

#include "../fib.h"
#include "router.h"

void
Router::InsertNamePrefix(Name *prefix, Bitmap *vector)
{
    fib_Insert(fib, prefix, vector);
}

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

void
Router::Run(void *arg)
{
    uint8_t nameBuffer[MAX_NAME_SIZE];
    for (int i = 0; i < numberOfNames; i++) {
        // Peek at the length of the name TLV
        if (read(sockfd, nameBufferbuf, 4, 0) < 0) {
            std::cerr << "failed to read the header of name " << i << " from the socket" << std::endl;
            return;
        }

        // Read the rest of the name
        uin16_t length = (((uint16_t)nameBuffer[2]) << 8) | (uint16_t)nameBuffer[3];
        if (read(sockfd, nameBufferbuf, length, 4) < 0) {
            std::cerr << "failed to read the body of name " << i << " from the socket" << std::endl;
            return;
        }

        // Reconstruct the name
        PARCBuffer *nameBuffer = parcBuffer_Wrap(nameBuffer, length + 4, 0, length + 4);
        Name *name = name_CreateFromBuffer(nameBuffer);

        // Index the name into the FIB
        Bitmap *output = fib_LPM(fib, name);
        // XXX: assert the outut is not NULL
        // XXX: use the output bitmap to send to the right socket(s)

        // Blindly write the name to the output socket
        if (write(sinkfd, nameBuffer, length + 4) < 0) {
            std::cerr << "failed to write name " << i << " to the sink socket" << std:endl;
        }
    }
}