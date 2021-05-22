#include "signals.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

const char LISTEN_ADDR [] = "0.0.0.0";
const unsigned short LISTEN_PORT = 21000;

int make_listener(const char* addr, const unsigned short port) {
    int listener = socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, IPPROTO_TCP);
    
    if (listener == -1) {
        perror("Failed to create listener socket");
        return -1;
    }
    
    int optval = 1;
    
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof optval) == -1) {
        perror("failed to set listener to SO_REUSEADDR");
        return -1;
    }
    
    struct sockaddr_in local_ep;
    local_ep.sin_family = AF_INET;
    local_ep.sin_port = htons(port);
    inet_aton(addr, (struct in_addr*)&local_ep.sin_addr.s_addr);
    bzero(&local_ep.sin_zero, sizeof local_ep.sin_zero);
    
    if (bind(listener, (struct sockaddr*) &local_ep, sizeof local_ep) != 0) {
        perror("listener failed to bind");
        return -1;
    }
    
    // listen
    if (listen(listener, 5) != 0) {
        perror("listener failed to listen");
        return -1;
    }
    
    printf("Created listener listening on %s:%u\n", addr, port);
    
    return listener;
}

int accept_client(const int listener) {
    fd_set readfds;
    int ret;

    printf("Waiting for a client to connect...\n");
    
    do {
        FD_ZERO(&readfds);
        FD_SET(listener, &readfds);
        
        ret = select(FD_SETSIZE, &readfds, 0, 0, 0);

        if (ret == -1) {
            perror("select");
            return -1;
        }

        if (shutting_down) {
            return -1;
        }
    } while (ret == 0);
    
    int sock_tcp = accept(listener, 0, 0);
    
    if (sock_tcp == -1) {
        perror("failed to accept connection");
        return -1;
    }
    
    return sock_tcp;
}

int make_sock_udp() {

}

void serve(const char* listen_addr, const unsigned short listen_port) {
    int listener = -1, sock_tcp = -1, sock_udp = -1;
    
    if ((listener = make_listener(listen_addr, listen_port)) == -1) {
        fprintf(stderr, "Failed to listen. quitting.\n");
        goto done;
    }
    
    if ((sock_tcp = accept_client(listener)) == -1) {
        fprintf(stderr, "Nothing to accept. quitting.\n");
        goto done;
    }
    
    while (!shutting_down) {
        unsigned price = rand();
        printf("Randomly generated price: %u\n", price);
        
        // Send price to TCP client.
        if (send(sock_tcp, &price, sizeof(price), 0) == -1) {
            perror("send");
            break;
        }
        
        // Send price to UDP client.
        //sendto(sock_udp, &price, sizeof(price), 0, &dest, )
        
        sleep(1);
    }
    
done:
    printf("Shutting down...");
    if (sock_udp != -1) close(sock_udp);
    if (sock_tcp != -1) close(sock_tcp);
    if (listener != -1) close(listener);
}

int main(int argc, char** argv)
{
    printf("Price Server\n");
    
    srand(time(0));
    
    if (setup_signal_handlers() != 0) {
        fprintf(stderr, "Error setting up signal handlers");
        return EXIT_FAILURE;
    }
    
    serve(LISTEN_ADDR, LISTEN_PORT);
    
    return EXIT_SUCCESS;
}
