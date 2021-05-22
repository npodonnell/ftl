#include <fcntl.h>

int set_blocking(int sockfd, int blocking) {
    int flags = fcntl(socketfd, F_GETFL, 0);
    
    if (blocking) {
        flags &= ~O_NONBLOCK;
    } else {
        flags |= O_NONBLOCK;
    }
    
    if (fcntl(socketfd, F_SETFL, flags) == -1){
        perror("calling fcntl");
    }
}
