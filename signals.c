#include <signal.h>
#include <stdio.h>

int shutting_down = 0;

static void handle_signal(const int signal) {
    printf("caught signal %d\n", signal);
    
    if (signal != SIGPIPE) {
        shutting_down = 1;
    }
}

int setup_signal_handlers() {
    struct sigaction sa;
    
    sa.sa_handler = &handle_signal;
    sa.sa_flags = SA_RESTART;
    
    sigfillset(&sa.sa_mask);
    
    if (sigaction(SIGINT, &sa, 0) == -1) {
        perror("Error: cannot handle SIGINT");
        return -1;
    }
    
    if (sigaction(SIGHUP, &sa, 0) == -1) {
        perror("Error: cannot handle SIGHUP");
        return -1;
    }
    
    if (sigaction(SIGTERM, &sa, 0) == -1) {
        perror("Error: cannot handle SIGTERM");
        return -1;
    }
    
    if (sigaction(SIGPIPE, &sa, 0) == -1) {
        perror("Error: cannot handle SIGPIPE");
        return -1;
    }
    
    return 0;
}

