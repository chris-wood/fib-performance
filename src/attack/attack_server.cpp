#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>

#include "../timer.h"
#include "attack_server.h"

#define MAX_NAME_SIZE sizeof(uint16_t)

void
AttackServer::Run()
{
    uint8_t nameBuffer[MAX_NAME_SIZE];
    for (int i = 0; i < numberOfNames; i++) {
        std::cout << "server processing name " << i << std::endl;

        // Peek at the length of the name TLV
        if (read(sockfd, nameBuffer, 2) < 0) {
            std::cerr << "failed to read the header of name " << i << " from the socket" << std::endl;
            return;
        }

        // Read the rest of the name
        uint16_t length = (((uint16_t)nameBuffer[0]) << 8) | (uint16_t)nameBuffer[1];
        if (read(sockfd, nameBuffer, length) < 0) {
            std::cerr << "failed to read the body of name " << i << " from the socket" << std::endl;
            return;
        }

        struct timespec now = timerStart();
        times.push_back(now);
    }
}

void *
runServer(void *arg)
{
    AttackServer *server = (AttackServer *) arg;
    server->Run();
    return NULL;
}