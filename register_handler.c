#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_USERNAME 100
#define MAX_PASSWORD 100
#define USER_FILE "users.txt"

typedef struct User {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    struct User* next;
} User;

User* load_users() {
    FILE* file = fopen(USER_FILE, "r");
    if (!file) return NULL;

    User* head = NULL;
    User* tail = NULL;
    char username[MAX_USERNAME], password[MAX_PASSWORD];

    while (fscanf(file, "%s %s", username, password) == 2) {
        User* new_user = (User*)malloc(sizeof(User));
        if (!new_user) {
            fclose(file);
            return NULL;
        }
        strcpy(new_user->username, username);
        strcpy(new_user->password, password);
        new_user->next = NULL;

        if (!head) {
            head = new_user;
            tail = new_user;
        } else {
            tail->next = new_user;
            tail = new_user;
        }
    }

    fclose(file);
    return head;
}

int username_exists(User* head, const char* username) {
    User* current = head;
    while (current) {
        if (strcmp(current->username, username) == 0) {
            return 1;
        }
        current = current->next;
    }
    return 0;
}

void save_user(const char* username, const char* password) {
    FILE* file = fopen(USER_FILE, "a");
    if (!file) {
        return;
    }
    fprintf(file, "%s %s\n", username, password);
    fclose(file);
}

void free_users(User* head) {
    while (head) {
        User* temp = head;
        head = head->next;
        free(temp);
    }
}

void url_decode(char *src) {
    char *dest = src;
    char hex[3];
    while (*src) {
        if (*src == '%') {
            hex[0] = *(src + 1);
            hex[1] = *(src + 2);
            hex[2] = '\0';
            *dest++ = (char) strtol(hex, NULL, 16);
            src += 3;
        } else if (*src == '+') {
            *dest++ = ' ';
            src++;
        } else {
            *dest++ = *src++;
        }
    }
    *dest = '\0';
}

void get_form_data(char* username, char* password) {
    int len = atoi(getenv("CONTENT_LENGTH"));
    char *data = (char*)malloc(len + 1);
    if (!data) {
        printf("Content-type: text/plain\n\n");
        printf("Error: Memory allocation failed.\n");
        exit(1);
    }

    fread(data, 1, len, stdin);
    data[len] = '\0';

    sscanf(data, "username=%99[^&]&password=%99[^\n]", username, password);

    url_decode(username);
    url_decode(password);

    free(data);
}

int main() {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];

    printf("Content-Type: text/html\n\n");

    get_form_data(username, password);

    User* users = load_users();

    if (username_exists(users, username)) {
        printf("<script>alert('Username already exists. Please choose a different username.'); window.location.href='../index.html';</script>");
        free_users(users);
        return 0;
    }

    save_user(username, password);

    printf("<script>window.location.href='../index.html?registered=success';</script>");

    free_users(users);
    return 0;
}
