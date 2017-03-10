#ifndef ROUTER_H_
#define ROUTER_H_

#include "../fib.h"
#include "../name.h"
#include "../bitmap.h"

class Router
{
public:
    Router(FIB *theFib) {
        fib = theFib;
    }

    void InsertNamePrefix(Name *prefix, Bitmap *vector);

    void ConnectSource(int sock);
    void ConnectSink(int sock);

    void SetNumberOfNames(int num) {
        numberOfNames = num;
    }

    void Run();
private:
    FIB *fib;
    int numberOfNames;
    int sourcefd;
    int sinkfd;
};

void *runRouter(void *arg);

#endif // ROUTER_H_