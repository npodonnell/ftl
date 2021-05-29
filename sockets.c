#include <netinet/in.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>

int set_blocking(int sockfd, int blocking) {
    int flags;
    
    if ((flags = fcntl(sockfd, F_GETFL, 0)) == -1) {
        perror("fcntl");
        return -1;
    }
    
    if (blocking) {
        flags &= ~O_NONBLOCK;
    } else {
        flags |= O_NONBLOCK;
    }
    
    if (fcntl(sockfd, F_SETFL, flags) == -1){
        perror("fcntl");
        return -1;
    }
    
    return 0;
}

int interruptable_recvfrom(int sockfd,
                           void* buf,
                           size_t len,
                           int flags,
                           struct sockaddr* src_addr,
                           socklen_t* addrlen) {
    fd_set readfds;
    
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    
    int ret = select(FD_SETSIZE, &readfds, 0, 0, 0);
    
    if (ret == -1) {
        perror("select");
        return -1;
    }
    
    if (ret == 0) {
        return 0;
    }
    
    return recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
}
