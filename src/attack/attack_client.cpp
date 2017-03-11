//
// Created by Christopher Wood on 3/5/17.
//

#include <vector>
#include <iostream>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../name.h"
#include "../timer.h"
#include "attack_client.h"

int
AttackClient::LoadNameList(char *nameFile)
{
    // XXX: read the name list from the file

    return 0;
}

void
AttackClient::Run()
{
    // TODO(caw): finish me
    for (std::vector<Name *>::iterator itr = names.begin(); itr != names.end(); itr++) {
        Name *name = *itr;

        // Serialize and send the name
        PARCBuffer *nameWireFormat = name_GetWireFormat(name, name_GetSegmentCount(name));
        send(sockfd, parcBuffer_Overlay(nameWireFormat, 0), parcBuffer_Remaining(nameWireFormat), 0);

        // Record the time it was sent
        struct timespec start = timerStart();
        times.push_back(start);
    }
}

void *
runClient(void *arg)
{
    AttackClient *client = (AttackClient *) arg;
    client->Run();
    return NULL;
}