#include <stdio.h>

int main() {
    FILE *fp = fopen("C:\\xampp\\htdocs\\admin_messages.txt", "w");
    if (fp != NULL) {
        fclose(fp);
        printf("Content-Type: text/plain\n\n");
        printf("All messages deleted.\n");
    } else {
        printf("Content-Type: text/plain\n\n");
        printf("Error: Could not delete messages.\n");
    }
    return 0;
}
