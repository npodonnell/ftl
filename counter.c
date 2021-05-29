#include "signals.h"
#include "sockets.h"
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

const char* PRICE_SERVER_IP = "127.0.0.1";
const unsigned short PRICE_SERVER_PORT = 21123;

const char PULL_LISTEN_IP [] = "127.0.0.1";
const unsigned short PULL_LISTEN_PORT = 21124;

int make_getter(const char* ip, const unsigned short port) {
    int getter;
    struct sockaddr_in addr;
    
    getter = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    make_addr(ip, port, &addr);
    
    if (connect(getter, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect");
        return -1;
    }
    set_blocking(getter, 0);
    
    return getter;
}

int make_listener(const char* ip, const unsigned short port) {
    int listener;
    struct sockaddr_in addr;
    
    listener = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    make_addr(ip, port, &addr);
    
    if (bind(listener, (struct sockaddr*) &addr, sizeof(struct sockaddr_in)) != 0) {
        perror("listener failed to bind");
        return -1;
    }
    
    printf("Created UDP listener listening on %s:%u\n", ip, port);
    return listener;
}

int ep_init(int nsocks, ...) {
    int efd = epoll_create1(0);
    va_list valist;
    va_start(valist, nsocks);
    
    for (int i = 0;  i < nsocks; i++) {
        int fd = va_arg(valist, int);
        printf("Adding %d to epoll\n", fd);
        
        struct epoll_event event;
        
        event.data.u64 = (uint64_t)0;
        event.data.fd = fd;
        event.events = EPOLLET | EPOLLIN | EPOLLOUT | EPOLLHUP;
        
        if (epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event) == -1) {
            perror("epoll_ctl");
            close(efd);
            return -1;
        }
    }
    
    va_end(valist);
    return efd;
}

void ep_cleanup(int efd, int nsocks, ...) {
    va_list valist;
    va_start(valist, nsocks);
    
    for (int i = 0;  i < nsocks; i++) {
        int fd = va_arg(valist, int);
        if (fd >= 0) {
            printf("Removing %d from epoll\n", fd);
            struct epoll_event dummy;
            if (epoll_ctl(efd, EPOLL_CTL_DEL, fd, &dummy) == -1) {
                fprintf(stderr, "Failure when removing %d from epoll instance", fd);
            }
        }
    }

    va_end(valist);
    close(efd);
}

void count() {
    int sock_tcp = -1, sock_udp = -1, efd = -1;
    struct epoll_event events[4];
    unsigned price;
    
    if ((sock_tcp = make_getter(PRICE_SERVER_IP, PRICE_SERVER_PORT)) < 0) {
        fprintf(stderr, "can not connect to price server\n");
        goto done;
    }
    
    if ((sock_udp = make_listener(PULL_LISTEN_IP, PULL_LISTEN_PORT)) < 0) {
        fprintf(stderr, "can not connect to price server\n");
        goto done;
    }
    
    if ((efd = ep_init(2, sock_tcp, sock_udp)) == -1) {
        fprintf(stderr, "ep_init failed");
        goto done;
    }
    
    while (!shutting_down) {
        int nevents = epoll_wait(efd, events, 4, -1);
        
        if (nevents == -1) {
            break;
        }
    
        printf("Got %d epoll events\n", nevents);
        
        for (int i = 0; i < nevents; i++) {
            int fd = events[i].data.fd;
            
            if (fd == sock_tcp) {
                puts("TCP");
                ssize_t nrecv = recv(sock_tcp, &price, sizeof(price), 0);
    
                if (nrecv < 0) {
                    perror("recv");
                    break;
                }
    
                if (nrecv == 0) {
                    printf("recv returned 0\n");
                    break;
                }
                
                printf("TCP received price: %u\n", price);
            } else if (fd == sock_udp) {
                puts("UDP");
                ssize_t nrecvfrom = recvfrom(sock_udp, &price, sizeof(price), 0, 0, 0);
                
                if (nrecvfrom < 0) {
                    perror("recvfrom");
                    break;
                }
    
                if (nrecvfrom != sizeof(price)) {
                    printf("recvfrom returned 0\n");
                    break;
                }
    
                printf("UDP received price: %u\n", price);
            }
        }
    }
    
done:
    ep_cleanup(efd, 2, sock_tcp, sock_udp);
    close(sock_udp);
    close(sock_tcp);
}

int main(int argc, char** argv) {
    printf("FTL Counter\n");
    
    if (setup_signal_handlers() != 0) {
        fprintf(stderr, "Error setting up signal handlers");
        return EXIT_FAILURE;
    }
    
    count();
    return EXIT_SUCCESS;
}
