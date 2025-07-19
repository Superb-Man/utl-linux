#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sched.h>

void sigterm_handler(int signum) {
    printf("[PID %d] Received SIGTERM. Cleaning up before exiting...\n", getpid());
    fflush(stdout);
    exit(0);
}
int main() {
    pid_t pid = fork();

    if (pid != 0)   printf("[PARENT] PID: %d - Forked child with PID: %d\n", getpid(), pid);
    else printf("[CHILD] PID: %d - Running...\n", getpid());

    if (pid == 0) { // child proc 
        signal(SIGTERM, sigterm_handler);
        printf("[CHILD] PID: %d - Waiting for SIGTERM...\n", getpid());
        while (1) {
            sleep(1);
        }
    } 
    else if (pid > 0) { // parent proc
        printf("[PARENT] PID: %d - Sleeping for 3 seconds...\n", getpid());
        sleep(3);
        
        printf("[PARENT] Sending SIGTERM to child (PID %d)...\n", pid);
        kill(pid, SIGTERM);

        sleep(3);
        printf("[PARENT] Relaunching child and sending SIGKILL...\n");

        pid = fork();
        if (pid != 0) {
            printf("[PARENT] PID: %d - Forked new child with PID: %d\n", getpid(), pid);
        }
        else {
            printf("[CHILD] PID: %d - Running...\n", getpid());
        }
        if (pid == 0) { // new child proc
            signal(SIGTERM, sigterm_handler);
            printf("[CHILD] PID: %d - Waiting for SIGKILL...\n", getpid());
            while (1) {
                sleep(1);
            }
        } 
        else {
            sleep(3);
            printf("[PARENT] Sending SIGKILL to child (PID %d)...\n", pid);
            kill(pid, SIGKILL); 
        }
    } 
    else {
        perror("fork");
        exit(1);
    }

    sleep(2);
    printf("[PARENT] Done.\n");
    return 0;
}
