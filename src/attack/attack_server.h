#ifndef ATTACK_SERVER_H_
#define ATTACK_SERVER_H_

#include <vector>

class AttackServer
{
public:
    AttackServer(int sock) {
        sockfd = sock;
    }

    void Run(void *arg);
    void SetNumberOfNames(int num) {
        numberOfNames = num;
    }
private:
    int sockfd;
    int numberOfNames;
    std::vector<struct timespec> times;
};

#endif // ATTACK_SERVER_H_