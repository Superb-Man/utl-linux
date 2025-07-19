#include <stdio.h>
#include <signal.h>
#include <stdlib.h>


static int counter = 0;
int printLk(int signum) {
    const char* str = "I love lara Khanom";
    printf("%s", str);
    counter++;
    printf(" (Counter: %d)\n", counter);
    return counter * 2;
}

void prints(int signum) {
    printf("Signal received in different context: %d\n", signum);
    counter++;
    printf(" (Counter: %d)\n", counter);

}

void shutDownProcess(int signum) {
    printf("Shutting down processes...\n");
    exit(0);
}
void (*sigint_handlers[])(int) = { (void (*)(int))printLk, prints };
size_t sigint_handler_count = sizeof(sigint_handlers) / sizeof(sigint_handlers[0]);

void interrupt(int signum, void (*handler)(int)) {
    signal(signum, handler);
    printf("Signal %d registered with handler %p\n", signum, (void*)handler);
}

int main() {
    interrupt(SIGINT, sigint_handlers[0]);
    signal(SIGTERM, shutDownProcess);
    int b = 0;
    // signal(SIGKILL, shutDownProcess); // Note: SIGKILL cannot be caught or ignored
    while (1) {
        printf("Running...\n");
        if (counter %4 == 2 && !b) {
            interrupt(SIGINT, sigint_handlers[0]); // automatically raises SIGINT after 3 counts. No need to press Ctrl+C for counter value to be 4
            raise(SIGINT);
            printf("SIGINT raised\n");
            b = 1; // Prevents multiple raises of SIGINT
        }
        else if (b == 1) {
            interrupt(SIGINT, sigint_handlers[1]);
            raise(SIGINT);
            printf("SIGINT raised again\n");
            b = 0;
            interrupt(SIGINT, sigint_handlers[0]);
        }
        if (counter >= 15) {
            printf("Counter reached 15, sending SIGTERM to self...\n");
            raise(SIGTERM);
        }
        if (counter >= 50) {
            printf("Counter reached 50, exiting...\n");
            break;
        }
        sleep(1);
    }
    return 0;
}