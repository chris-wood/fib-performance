#ifndef ATTACK_CLIENT_H_
#define ATTACK_CLIENT_H_

#include <vector>
#include "../name_reader.h"

using namespace std;

class AttackClient
{
    public:
    AttackClient(int sock) {
        sockfd = sock;
    }

    int LoadNames(NameReader *reader);

    void Run();

    std::vector<Name *> names;
    std::vector<struct timespec> times;
    int sockfd;
    int numNames;
    char *prefix;
};

void *runClient(void *arg);

#endif // ATTACK_CLIENT_H_