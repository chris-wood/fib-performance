//
// Created by Christopher Wood on 3/5/17.
//

#include <vector>

#include "../name.h"
#include "attack_client.h"

int
AttackClient::LoadNameList(char *nameFile)
{
    // XXX: read the name list from the file

    return 0;
}

void
AttackClient::Run(void *arg)
{
    // TODO(caw): finish me
    for (std::vector<Name *>::iterator itr = names.begin(); itr != names.end(); itr++) {
        Name *name = *itr;

        // Serialize and send the name
        PARCBuffer *nameWireFormat = name_GetWireFormat(name, name_GetSegmentCount(name));
        send(sock, parcBuffer_Overlay(nameWireFormat), parcBuffer_Remaining(nameWireFormat), 0);

        // Record the time it was sent
        struct timespec start = timerStart();
        times.push_back(start);
    }
}