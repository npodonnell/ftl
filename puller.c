#include "signals.h"
#include "sockets.h"
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

const char LISTEN_IP [] = "127.0.0.1";
const unsigned short LISTEN_PORT = 21124;

int make_listener(const char* addr, const unsigned short port) {
    int listener = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    struct sockaddr_in local_ep;
    bzero(&local_ep, sizeof(local_ep));
    local_ep.sin_family = AF_INET;
    local_ep.sin_port = htons(port);
    inet_aton(addr, (struct in_addr*)&local_ep.sin_addr.s_addr);
    
    if (bind(listener, (struct sockaddr*) &local_ep, sizeof local_ep) != 0) {
        perror("listener failed to bind");
        return -1;
    }
    
    printf("Created UDP listener listening on %s:%u\n", addr, port);
    
    return listener;
}

int main(int argc, char** argv) {
    printf("Puller\n");
    int listener;
    unsigned price;
    
    listener = make_listener(LISTEN_IP, LISTEN_PORT);
    
    if (setup_signal_handlers() != 0) {
        fprintf(stderr, "Error setting up signal handlers");
        return EXIT_FAILURE;
    }
    
    while(!shutting_down) {
        ssize_t recvfrom_ret = interruptable_recvfrom(listener, &price, sizeof(price), 0, 0, 0);
        if (recvfrom_ret == -1) {
            perror("recvfrom");
            break;
        }
    
        if (recvfrom_ret != sizeof(price)) {
            printf("recvfrom returned 0\n");
            break;
        }
        
        printf("pulled: price: %u\n", price);
    }
    
    close(listener);
    
    return EXIT_SUCCESS;
}
