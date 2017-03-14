//
// Created by Christopher Wood on 3/5/17.
//

#include "router.h"
#include "attack_client.h"
#include "attack_server.h"

#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_BufferComposer.h>
#include <parc/statistics/parc_BasicStats.h>

#include "../fib.h"
#include "../bitmap.h"
#include "../timer.h"
#include "../fib_cisco.h"
#include "../sha256hasher.h"

#include <sys/socket.h>
#include <fcntl.h>
#include <iostream>
#include <pthread.h>

using namespace std;

void
ProcessResults(Router *router)
{
    for (int i = 0; i < router->inTimes.size(); i++) {
        struct timespec start = router->inTimes.at(i);
        struct timespec end = router->outTimes.at(i);
        std::cout << i << "," << timeDelta(start, end) << std::endl;
    }
}

static void
usage()
{
    std::cout << "usage: drive <load_file> <test_file>" << std::endl;
}

int
main(int argc, char **argv)
{
    if (argc < 3) {
        usage();
        exit(-1);
    }

    // Create the router FIB
    FIBCisco *ciscoFIB = fibCisco_Create(3);
    FIB *fib = fib_Create(ciscoFIB, CiscoFIBAsFIB);

    int hashed = argc == 3 ? 0 : atoi(argv[3]);
    Hasher *hasher = NULL;
    if (hashed == 1) {
        SHA256Hasher *sha256hasher = sha256hasher_Create();
        hasher = hasher_Create(sha256hasher, SHA256HashAsHasher);
    }

    // Create the router and populate it with names
    Router *router = new Router(fib);
    NameReader *loadReader = nameReader_CreateFromFile(argv[1], NULL);
    int fibSize = router->LoadHashedNames(loadReader, hasher);
    std::cerr << "Loaded " << fibSize << " prefixes into the FIB" << std::endl;

    NameReader *reader = nameReader_CreateFromFile(argv[2], NULL);
    int numberOfNames = router->LoadHashedTestNames(reader, hasher);
    std::cerr << "Processing " << numberOfNames << " through the pipe" << std::endl;

    // Run the router -- process inputs and outputs in separate threads
    pthread_t inputThread;
    int result;
    result = pthread_create(&inputThread, NULL, processInputs, router);
    if (result != 0) {
        std::cerr << "Unable to create the input thread" << std::endl;
    }

    result = pthread_create(&inputThread, NULL, runRouter, router);
    if (result != 0) {
        std::cerr << "Unable to create the router thread" << std::endl;
    }

    void *status;
    pthread_join(inputThread, &status);

    // Compute the per-packet throughput
    ProcessResults(router);


//    // Create the upstream socket pair
//    int sinksockets[2] = {0};
//    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sinksockets) < 0) {
//        std::cerr << "Failed to create the sink socket pair" << std::endl;
//        return -1;
//    }
//    int flags = fcntl(sinksockets[0], F_GETFL, 0);
//    flags |= O_NONBLOCK;
//    fcntl(sinksockets[0], F_SETFL, flags);
//    flags = fcntl(sinksockets[1], F_GETFL, 0);
//    flags |= O_NONBLOCK;
//    fcntl(sinksockets[1], F_SETFL, flags);
//
//    // Create the server
//    AttackServer *server = new AttackServer(sinksockets[0]);
//
//    // Create the BitVector corresponding to this socket
//    // The size of the output bitmap is not important
//    Bitmap *outputMap = bitmap_Create(16);
//    bitmap_Set(outputMap, 1);
//
//    // Create the source socket
//    int sourcesockets[2] = {0};
//    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sourcesockets) < 0) {
//        std::cerr << "Failed to create the source socket pair" << std::endl;
//        return -1;
//    }
//    flags = fcntl(sourcesockets[0], F_GETFL, 0);
//    flags |= O_NONBLOCK;
//    fcntl(sourcesockets[0], F_SETFL, flags);
//    flags = fcntl(sourcesockets[1], F_GETFL, 0);
//    flags |= O_NONBLOCK;
//    fcntl(sourcesockets[1], F_SETFL, flags);
//
//    // Create the client and build the name list
//    AttackClient *client = new AttackClient(sourcesockets[0]);
//
//    // Connect the router to the sockets
//    router->ConnectSource(sourcesockets[1]);
//    router->ConnectSink(sinksockets[1]);
//
//    // Set the name limit on the router and server
//    router->SetNumberOfNames(numberOfNames);
//    server->SetNumberOfNames(numberOfNames);
//
//    std::cerr << "Routers loaded -- starting the threads" << std::endl;
//
//    // Create threads for the client, server, and router
//    // TODO(cawood): maybe pass the run functions files that are used to write results when done?
//    pthread_t serverThread;
//    int result;
//    result = pthread_create(&serverThread, NULL, runServer, server);
//    if (result != 0) {
//        std::cerr << "Unable to create the server thread" << std::endl;
//    }
//    pthread_t routerThread;
//    result  = pthread_create(&routerThread, NULL, runRouter, router);
//    if (result != 0) {
//        std::cerr << "Unable to create the router thread" << std::endl;
//    }
//    pthread_t clientThread;
//    result = pthread_create(&clientThread, NULL, runClient, client);
//    if (result != 0) {
//        std::cerr << "Unable to create the client thread" << std::endl;
//    }
//
//    // Run the server to completion -- the server can't complete until both the client and router are done
//    void *status;
//    result = pthread_join(clientThread, &status);
//    if (result != 0) {
//        std::cerr << "Unable to join on the client thread" << std::endl;
//    }
//    std::cerr << "Client is done." << std::endl;
//    result = pthread_join(routerThread, &status);
//    if (result != 0) {
//        std::cerr << "Unable to join on the router thread" << std::endl;
//    }
//    std::cerr << "Router is done." << std::endl;
//    result = pthread_join(serverThread, &status);
//    if (result != 0) {
//        std::cerr << "Unable to join on the server thread" << std::endl;
//    }
//    std::cerr << "Server is done." << std::endl;
//
//    // Compute the per-packet throughput
//    ProcessResults(client, server);

//    // Cleanup and exit
//    pthread_exit(NULL);

    return 0;
}
