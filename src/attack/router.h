#ifndef ROUTER_H_
#define ROUTER_H_

#include "../fib.h"
#include "../name.h"
#include "../bitmap.h"
#include "../name_reader.h"

#include <vector>

using namespace std;

class Router
{
public:
    Router(FIB *theFib) {
        fib = theFib;
    }

    int LoadNames(NameReader *reader);
    int LoadTestNames(NameReader *reader);

    int LoadHashedNames(NameReader *reader, Hasher *hasher);
    int LoadHashedTestNames(NameReader *reader, Hasher *hasher);

    void ConnectSource(int sock);
    void ConnectSink(int sock);

    void SetNumberOfNames(int num) {
        numberOfNames = num;
    }

    void Run();
    void ProcessInputs();

    std::vector<Name *> names;
    std::vector<struct timespec> inTimes;
    std::vector<struct timespec> outTimes;

    FIB *fib;
    int numberOfNames;
    int sourcefd;
    int sinkfd;
};

void *processInputs(void *arg);
void *runRouter(void *arg);

#endif // ROUTER_H_