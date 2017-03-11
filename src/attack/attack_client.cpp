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
AttackClient::LoadNames(NameReader *reader)
{
    while (nameReader_HasNext(reader)) {
        Name *name = nameReader_Next(reader);
        names.push_back(name);
    }
    return names.size();
}

void
AttackClient::Run()
{
    int i = 0;
    for (std::vector<Name *>::iterator itr = names.begin(); itr != names.end(); itr++) {
        std::cout << "client processing name " << i++ << std::endl;

        Name *name = *itr;

        // Serialize and send the name
        PARCBuffer *nameWireFormat = name_GetWireFormat(name, name_GetSegmentCount(name));
        uint8_t *nameBuffer = (uint8_t *) parcBuffer_Overlay(nameWireFormat, 0);
        size_t nameLength = parcBuffer_Remaining(nameWireFormat);

        for (size_t i = 0; i < nameLength; i++) {
            printf("%02x ", nameBuffer[i]);
        }
        printf("\n");

        send(sockfd, (void *) nameBuffer, nameLength, 0);

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