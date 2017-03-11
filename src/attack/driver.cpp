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

#include <sys/socket.h>
#include <iostream>
#include <pthread.h>

using namespace std;

void
ProcessResults(AttackClient *client, AttackServer *server)
{
    for (int i = 0; i < client->times.size(); i++) {
        struct timespec start = client->times.at(i);
        struct timespec end = server->times.at(i);
        std::cout << i << "," << timeDelta(start, end) << std::endl;
    }
}

static void
usage()
{
    printf("usage: drive <load_file> <test_file>");
}

int
main(int argc, char **argv)
{
    if (argc != 3) {
        usage();
        exit(-1);
    }

    // Create the router FIB
    FIBCisco *ciscoFIB = fibCisco_Create(3);
    FIB *fib = fib_Create(ciscoFIB, CiscoFIBAsFIB);

    // Create the router and populate it with names
    Router *router = new Router(fib);
    NameReader *loadReader = nameReader_CreateFromFile(argv[1], NULL);
    router->LoadNames(loadReader);

    // Create the upstream socket pair
    int sinksockets[2] = {0};
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sinksockets) < 0) {
        std::cerr << "Failed to create the sink socket pair" << std::endl;
        return -1;
    }

    // Create the server
    AttackServer *server = new AttackServer(sinksockets[0]);

    // Create the BitVector corresponding to this socket
    // The size of the output bitmap is not important
    Bitmap *outputMap = bitmap_Create(16);
    bitmap_Set(outputMap, 1);

    // Create the source socket
    int sourcesockets[2] = {0};
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sourcesockets) < 0) {
        std::cerr << "Failed to create the source socket pair" << std::endl;
        return -1;
    }

    // Create the client and build the name list
    AttackClient *client = new AttackClient(sourcesockets[0]);
    NameReader *reader = nameReader_CreateFromFile(argv[2], NULL);
    int numberOfNames = client->LoadNames(reader);

    // Connect the router to the sockets
    router->ConnectSource(sourcesockets[1]);
    router->ConnectSink(sinksockets[1]);

    // Set the name limit on the router and server
    router->SetNumberOfNames(numberOfNames);
    server->SetNumberOfNames(numberOfNames);

    std::cout << "Routers loaded -- starting the threads" << std::endl;

    // Create threads for the client, server, and router
    // TODO(cawood): maybe pass the run functions files that are used to write results when done?
    pthread_t serverThread;
    int result;
    result = pthread_create(&serverThread, NULL, runServer, server);
    if (result != 0) {
        std::cerr << "Unable to create the server thread" << std::endl;
    }
    pthread_t routerThread;
    result  = pthread_create(&routerThread, NULL, runRouter, router);
    if (result != 0) {
        std::cerr << "Unable to create the router thread" << std::endl;
    }
    pthread_t clientThread;
    result = pthread_create(&clientThread, NULL, runClient, client);
    if (result != 0) {
        std::cerr << "Unable to create the client thread" << std::endl;
    }

    // Run the client to completion
    void *status;
    result = pthread_join(serverThread, &status);
    if (result != 0) {
        std::cerr << "Unable to join on the server thread" << std::endl;
    }

    // Compute the per-packet throughput
    ProcessResults(client, server);

    // Cleanup and exit
    pthread_exit(NULL);

    return 0;
}
