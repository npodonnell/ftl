#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

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

void cleanup() {
    printf("Cleaning up...");
}

int main(int argc, char** argv) {
    printf("FTL Counter\n");
    
    if (setup_signal_handlers() != 0) {
        fprintf(stderr, "Error setting up signal handlers");
        return EXIT_FAILURE;
    }
    
    while (!shutting_down) {
        printf("wait\n");
        sleep(5);
    }
    
    cleanup();
    
    return EXIT_SUCCESS;
}
