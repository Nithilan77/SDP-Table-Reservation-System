#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

// verify_session is provided by utils.h

int main() {
    if (!verify_session()) {
        printf("Status: 403 Forbidden\nContent-Type: text/plain\n\nError: Unauthorized\n");
        return 0;
    }
    acquire_lock();
    FILE *fp = fopen("messages.txt", "w");
    if (fp != NULL) {
        fclose(fp);
        release_lock();
        printf("Content-Type: text/plain\n\n");
        printf("All messages deleted.\n");
    } else {
        release_lock();
        printf("Content-Type: text/plain\n\n");
        printf("Error: Could not delete messages.\n");
    }
    return 0;
}
