#include "signals.h"
#include "pass-fd.h"
#include "sockets.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

const char LISTEN_ADDR [] = "127.0.0.1";
const unsigned short LISTEN_PORT = 21123;

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
    bzero(&local_ep, sizeof(local_ep));
    local_ep.sin_family = AF_INET;
    local_ep.sin_port = htons(port);
    inet_aton(addr, (struct in_addr*)&local_ep.sin_addr.s_addr);

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

size_t add_client(int** pclients, size_t nclients, int new_client, int ifree) {
    if (ifree == -1) {
        ifree = nclients;
        (*pclients) = (int*) realloc(*pclients, ++nclients * sizeof(int));
    }
    
    (*pclients)[ifree] = new_client;
    
    return nclients;
}

size_t remove_client(int* clients, size_t nclients, int iclient) {
    close(clients[iclient]);
    clients[iclient] = -1;
    
    if (nclients == iclient - 1) {
        nclients--;
    }
    
    return nclients;
}

void accept_loop(const char* listen_addr, const unsigned short listen_port, const int dispatcher) {
    int listener, client = -1;
    
    if ((listener = make_listener(listen_addr, listen_port)) == -1) {
        fprintf(stderr, "Failed to listen. quitting.\n");
        goto done;
    }
    
    while (!shutting_down) {
        puts("accept loop");
    
        if ((client = accept_client(listener)) == -1) {
            goto done;
        }
        
        send_fd(dispatcher, client);
    }
    
done:
    puts("End accept loop");
    close(listener);
}

void broadcast_loop(int collector) {
    srand(time(0));
    
    int* clients = 0;
    size_t nclients = 0;
    int new_client;
    int ifree = -1;
    
    while (!shutting_down) {
        if (nclients > 0) {
            unsigned price = rand();
            printf("Randomly generated price: %u\n", price);
            
            for (int i = 0; i < nclients; i++) {
                if (clients[i] != -1) {
                    // Send price to TCP client.
                    if (send(clients[i], &price, sizeof(price), 0) == -1) {
                        // Client is gone.
                        printf("Removing client %d\n", i);
                        nclients = remove_client(clients, nclients, i);
                        ifree = i;
                    }
                }
            }
        }
        
        // Check for new clients.
        puts("checking for new clients");
        
        if ((new_client = recv_fd(collector)) > -1) {
            printf("new client: %d\n", new_client);
            nclients = add_client(&clients, nclients, new_client, ifree);
            ifree = -1;
        }
        
        puts("finished checking for new clients");
    }
    
    if (nclients > 0) {
        free(clients);
    }
    
    puts("End broadcast");
}

int main(int argc, char** argv) {
    printf("Price Server\n");
    
    int xfer_fds[2];
    
    if (socketpair(AF_UNIX, SOCK_DGRAM, 0, xfer_fds) != 0) {
        perror("socketpair");
        return EXIT_FAILURE;
    }
    
    set_blocking(xfer_fds[0], 0);
    set_blocking(xfer_fds[1], 0);
    
    if (setup_signal_handlers() != 0) {
        fprintf(stderr, "Error setting up signal handlers");
        return EXIT_FAILURE;
    }
    
    if (fork() > 0) {
        accept_loop(LISTEN_ADDR, LISTEN_PORT, xfer_fds[0]);
        wait(0);
    } else {
        broadcast_loop(xfer_fds[1]);
    }
    
    return EXIT_SUCCESS;
}
