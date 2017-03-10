#ifndef ATTACK_CLIENT_H_
#define ATTACK_CLIENT_H_

class AttackClient
{
    public:
    AttackClient(int sock) {
        socket = sock;
    }

    int LoadNameList(char *nameFile);
    void Run(void *arg);

    private:

    std::vector<Name *> names;
    std::vector<struct timespec> times;
    int sock;
    int numNames;
    char *prefix;
};

#endif // ATTACK_CLIENT_H_