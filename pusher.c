#include "signals.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

const char* PRICE_SERVER_IP = "127.0.0.1";
const unsigned short PRICE_SERVER_PORT = 21123;

const char* PUSHER_IP = "127.0.0.1";
const unsigned short PUSHER_PORT = 21124;

void make_addr(const char* ip, const unsigned short port, struct sockaddr_in* out) {
    out->sin_family = AF_INET;
    out->sin_addr.s_addr = inet_addr(ip);
    out->sin_port = htons(port);
}


int main() {
    int reader, writer;
    struct sockaddr_in price_server_addr;
    struct sockaddr_in puller_addr;
    unsigned price;
    
    reader = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    writer = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    make_addr(PRICE_SERVER_IP, PRICE_SERVER_PORT, &price_server_addr);
    make_addr(PUSHER_IP, PUSHER_PORT, &puller_addr);
    
    if (connect(reader, (struct sockaddr*)&price_server_addr, sizeof(price_server_addr)) == -1) {
        perror("connect");
        return 1;
    }
    
    if (setup_signal_handlers() != 0) {
        fprintf(stderr, "Error setting up signal handlers");
        return EXIT_FAILURE;
    }
    
    while(!shutting_down) {
        int nrecv = recv(reader, &price, sizeof(price), 0);
        
        if (nrecv < 0) {
            perror("recv");
            break;
        }
        
        if (nrecv == 0) {
            printf("Received 0 bytes.\n");
            break;
        }
        
        int nsend = sendto(writer, &price, sizeof(price), 0, (const struct sockaddr *) &puller_addr, sizeof(puller_addr));
        
        if (nsend < 0) {
            perror("send");
            break;
        }
    
        if (nsend == 0) {
            printf("Sent 0 bytes.\n");
            break;
        }
        
        printf("Relayed price: %u\n", price);
    }
    
    close(writer);
    close(reader);
    
    return EXIT_SUCCESS;
}
