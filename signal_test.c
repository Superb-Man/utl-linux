#include <stdio.h>
#include <signal.h>
#include <stdlib.h>


static int counter = 0;
void printLk(int signum) {
    const char* str = "I love lara Khanom";
    printf("%s", str);
    counter++;
    printf(" (Counter: %d)\n", counter);
}

void shutDownProcess(int signum) {
    printf("Shutting down processes...\n");
    exit(0);
}

int main() {
    signal(SIGINT, printLk);
    signal(SIGTERM, shutDownProcess);
    // how the sigterm signal is handled when will be called
    signal(SIGQUIT, printLk);
    // signal(SIGQUIT, shutDownProcess);
    // signal(SIGKILL, shutDownProcess);
    while (1) {
        printf("Running...\n");
        if (counter >= 10) {
            printf("Counter reached 10, exiting...\n");
            break;
        }
        sleep(1);
    }
    return 0;
}