#include "../timer.h"

#define MAX_NAME_SIZE sizeof(uint16)

void
AttackServer::Run()
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