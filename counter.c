#include <sys/epoll.h>
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

int shutting_down = 0;

void handle_signal(const int signal) {
    printf("caught signal %d\n", signal);
    shutting_down = 1;
}

int setup_signal_handlers() {
    struct sigaction sa;
    
    sa.sa_handler = &handle_signal;
    sa.sa_flags = SA_RESTART;
    
    sigfillset(&sa.sa_mask);
    
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error: cannot handle SIGINT");
        return -1;
    }
    
    if (sigaction(SIGHUP, &sa, NULL) == -1) {
        perror("Error: cannot handle SIGHUP");
        return -1;
    }
    
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("Error: cannot handle SIGTERM");
        return -1;
    }
    
    return 0;
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
            fprintf(stderr, "Failure when adding %d to epoll instance", fd);
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
        printf("Removing %d from epoll\n", fd);
    
        struct epoll_event dummy;
    
        if (epoll_ctl(efd, EPOLL_CTL_DEL, fd, &dummy) == -1) {
            fprintf(stderr, "Failure when removing %d from epoll instance", fd);
        }
    }

    va_end(valist);
    
    close(efd);
}

void count() {
    int sock_tcp, sock_udp, efd;
    
    sock_tcp = socket(AF_INET, SOCK_STREAM, 0);
    sock_udp = socket(AF_INET, SOCK_DGRAM, 0);
    
    if ((efd = ep_init(2, sock_tcp, sock_udp)) != -1) {
        while (!shutting_down) {
            printf("wait\n");
            sleep(5);
        }
    }
    
    ep_cleanup(efd, 2, sock_tcp, sock_udp);
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
