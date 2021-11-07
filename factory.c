#include "network/tcp.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Factory takes exactly 1 command line argument: the ip address\n");
        exit(-1);
    }

    const char * skynetAddr = argv[1];
    printf("Starting factory! Broadcast IP address: %s\n", skynetAddr);

    ClientThreadData skynetClient;
    connect_to_tcp_server(skynetAddr, &skynetClient);

    return 0;
}