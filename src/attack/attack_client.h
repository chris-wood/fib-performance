#ifndef ATTACK_CLIENT_H_
#define ATTACK_CLIENT_H_

#include <vector>

using namespace std;

class AttackClient
{
    public:
    AttackClient(int sock) {
        socket = sock;
    }

    int LoadNameList(char *nameFile);

    void Run();

    private:

    std::vector<Name *> names;
    std::vector<struct timespec> times;
    int socket;
    int numNames;
    char *prefix;
};

void *runClient(void *arg);

#endif // ATTACK_CLIENT_H_