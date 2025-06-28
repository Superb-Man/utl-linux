#include <stdio.h>
#include "debug.h"

int main() {
    DEBUG_PRINT("This is a debug message: %d%d\n", 42, 23);
    ERROR_PRINT("This is an error message: %s\n", "Something went wrong");

    int x = 5;
    int y = 0;

    // This assertion will pass
    ASSERT(x > 0, "x should be positive, got %d", x);

    // This assertion will fail and terminate the program
    ASSERT(y != 0, "y should not be zero, got %d", y);

    DEBUG_PRINT("This will not be printed because of the failed assertion.\n");

    return 0;
}
