#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

void parse_contact_input(char *data, char *name, char *email, char *message) {
    char *token = strtok(data, "&");
    while (token != NULL) {
        if (strncmp(token, "name=", 5) == 0) strcpy(name, token + 5);
        else if (strncmp(token, "email=", 6) == 0) strcpy(email, token + 6);
        else if (strncmp(token, "message=", 8) == 0) strcpy(message, token + 8);
        token = strtok(NULL, "&");
    }
}

// url_decode is provided by utils.h
int main() {
    char input[MAX] = {0};
    char name[100], email[100], message[500];
    char *lenstr = getenv("CONTENT_LENGTH");

    printf("Content-Type: text/html\n\n");
    printf("<html><body style='font-family: Arial; padding: 20px;'>");

    if (!lenstr) {
        printf("<p>Error: No data received</p></body></html>");
        return 1;
    }

    int len = atoi(lenstr);
    if (len >= sizeof(input)) len = sizeof(input) - 1;
    fread(input, 1, len, stdin);
    input[len] = '\0';

    parse_contact_input(input, name, email, message);
    url_decode(name);
    url_decode(email);
    url_decode(message);

    // ✅ Save to messages.txt
    acquire_lock();
    FILE *fp = fopen("messages.txt", "a");
    if (fp) {
        fprintf(fp, "Name: %s\nEmail: %s\nMessage: %s\n---\n", name, email, message);
        fclose(fp);
    }
    release_lock();

    // ✅ Display confirmation
    printf("<h2>✅ Message Sent!</h2>");
    printf("<p><strong>Name:</strong> %s</p>", name);
    printf("<p><strong>Email:</strong> %s</p>", email);
    printf("<p><strong>Message:</strong> %s</p>", message);

    // ✅ Redirect after 3 seconds
    printf("<meta http-equiv='refresh' content='3;url=/contact.html'>");
    printf("<p>You will be redirected back to the Contact Page shortly...</p>");

    printf("</body></html>");
    return 0;
}
