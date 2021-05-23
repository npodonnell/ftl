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
