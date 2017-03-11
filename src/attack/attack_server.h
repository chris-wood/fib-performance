#ifndef ATTACK_SERVER_H_
#define ATTACK_SERVER_H_

using namespace std;

#include <vector>

class AttackServer
{
public:
    AttackServer(int sock) {
        sockfd = sock;
    }

    void Run();

    void SetNumberOfNames(int num) {
        numberOfNames = num;
    }

    int sockfd;
    int numberOfNames;
    std::vector<struct timespec> times;
};

void *runServer(void *arg);

#endif // ATTACK_SERVER_H_