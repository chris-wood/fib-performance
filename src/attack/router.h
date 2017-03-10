
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

    void Run(void *arg);
private:
    FIB *fib;
    int numberOfNames;
    int sourcefd;
    int sinkfd;
};