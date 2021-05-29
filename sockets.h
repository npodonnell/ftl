#ifndef FTL_SOCKETS_H
#define FTL_SOCKETS_H

#include <netinet/in.h>
#include <sys/socket.h>

int set_blocking(int sockfd, int blocking);
int interruptable_recvfrom(int sockfd,
                           void* buf,
                           size_t len,
                           int flags,
                           struct sockaddr* src_addr,
                           socklen_t* addrlen);

#endif //FTL_SOCKETS_H
