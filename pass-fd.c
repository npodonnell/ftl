#include <sys/socket.h>
#include <string.h>
#include <stdio.h>

int send_fd(int socket, int fd) {
    struct msghdr msg = { 0 };
    char buf[CMSG_SPACE(sizeof(fd))];
    memset(buf, '\0', sizeof(buf));
    struct iovec io = { .iov_base = "ABC", .iov_len = 3 };
    
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);
    
    struct cmsghdr * cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(fd));
    
    *((int *) CMSG_DATA(cmsg)) = fd;
    
    msg.msg_controllen = CMSG_SPACE(sizeof(fd));
    
    if (sendmsg(socket, &msg, 0) < 0) {
        perror("sendmsg");
        return -1;
    }
    
    return 0;
}

int recv_fd(int socket) {
    struct msghdr msg = {0};
    char m_buffer[256];
    struct iovec io = { .iov_base = m_buffer, .iov_len = sizeof(m_buffer) };
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    
    char c_buffer[256];
    msg.msg_control = c_buffer;
    msg.msg_controllen = sizeof(c_buffer);
    
    puts("before recvmsg");
    
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(socket, &readfds);
    
    struct timeval tv = {1, 0};
    
    if (select(FD_SETSIZE, &readfds, 0, 0, &tv) == -1) {
        perror("select");
        return -1;
    }
    
    if (!FD_ISSET(socket, &readfds)) {
        return -1;
    }
    
    if (recvmsg(socket, &msg, 0) < 0) {
        perror("recvmsg");
        return -1;
    }
    puts("after recvmsg");
    
    struct cmsghdr * cmsg = CMSG_FIRSTHDR(&msg);
    unsigned char * data = CMSG_DATA(cmsg);
    int fd = *((int*) data);
    
    return fd;
}
