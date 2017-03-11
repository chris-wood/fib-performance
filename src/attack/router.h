#ifndef ROUTER_H_
#define ROUTER_H_

#include "../fib.h"
#include "../name.h"
#include "../bitmap.h"
#include "../name_reader.h"

class Router
{
public:
    Router(FIB *theFib) {
        fib = theFib;
    }

    void LoadNames(NameReader *reader);

    void ConnectSource(int sock);
    void ConnectSink(int sock);

    void SetNumberOfNames(int num) {
        numberOfNames = num;
    }

    void Run();

    FIB *fib;
    int numberOfNames;
    int sourcefd;
    int sinkfd;
};

void *runRouter(void *arg);

#endif // ROUTER_H_