//
// Created by Christopher Wood on 3/5/17.
//

#include "router.h"
#include "attack_client.h"
#include "attack_server.h"

#include <pthread.h>

void
ProcessResults(AttackClient *client, AttackServer *server)
{
    // XXX: compute the time of send vs time of receive for each name
    // XXX: the difference will grow since R becomes saturated
}

int
main(int argc, char *argv)
{
    // Create the router
    Router *router = new Router();

    // Create the upstream socket pair
    // XXX: http://osr507doc.xinuos.com/en/netguide/dusockD.socketpairs_codetext.html
    int sinksockets[2] = {0};
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sinksockets) < 0) {
        std::cerr << "Failed to create the sink socket pair" << std::endl;
        return -1;
    }

    // Create the server
    AttackServer *server = new AttackServer(sinksockets[1]);

    // Create the BitVector corresponding to this socket
    // The size of the output bitmap is not important
    Bitmap *outputMap = bitmap_Create(16);
    bitmap_Set(outputMap, 1);

    // Use the load file to populate the router with names
    char *loadFile = argv[1];
    // XXX: insert each prefix into the router, with the bit vector from above

    // Create the source socket
    int sourcesockets[2] = {0};
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sourcesockets) < 0) {
        std::cerr << "Failed to create the source socket pair" << std::endl;
        return -1;
    }

    // Create the client and build the name list
    AttackClient *client = new AttackClient(sourcesockets[0]);
    int numberOfNames = client->oadNameList(argv[2]);

    // Set the name limit on the router and server
    router->SetNumberOfNames(numberOfNames);
    server->SetNumberOfNames(numberOfNames);

    // Create threads for the client, server, and router
    pthread_t serverThread;
    rc = pthread_create(&serverThread, NULL, server->Run(), NULL);
    pthread_t routerThread;
    rc = pthread_create(&routerThread, NULL, router->Run(), NULL);
    pthread_t clientThread;
    rc = pthread_create(&clientThread, NULL, client->Run(), NULL);

    // Run the client to completion
    void *status;
    int result = pthread_join(serverThread, &status);
    if (result != 0) {
        std::cerr << "Unable to join on the server thread" << std::endl;
    }

    // Compute the per-packet throughput
    ProcessResults(client, server);

    // Cleanup and exit
    pthread_exit(NULL);
}
