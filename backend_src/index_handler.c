#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"



typedef struct UserNode {
    char username[MAX];
    char password[MAX];
    struct UserNode *next;
} UserNode;

UserNode* create_user(const char *username, const char *password) {
    UserNode *newNode = (UserNode *)malloc(sizeof(UserNode));
    strcpy(newNode->username, username);
    strcpy(newNode->password, password);
    newNode->next = NULL;
    return newNode;
}

UserNode* load_users(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return NULL;

    UserNode *head = NULL, *tail = NULL;
    char user[MAX], pass[MAX];

    while (fscanf(file, "%s %s", user, pass) != EOF) {
        UserNode *newNode = create_user(user, pass);
        if (!head) {
            head = newNode;
            tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
    }
    fclose(file);
    return head;
}

void free_users(UserNode *head) {
    while (head) {
        UserNode *temp = head;
        head = head->next;
        free(temp);
    }
}

int authenticate(const char *role, const char *username, const char *password) {
    if (strcmp(role, "admin") == 0) {
        return (strcmp(username, "admin") == 0 && strcmp(password, "admin123") == 0);
    }

    UserNode *users = load_users("users.txt");
    if (!users) return 0;

    unsigned long pass_hash = simple_hash(password);

    UserNode *current = users;
    while (current) {
        unsigned long stored_hash = strtoul(current->password, NULL, 10);
        if (strcmp(current->username, username) == 0 && stored_hash == pass_hash) {
            free_users(users);
            return 1;
        }
        current = current->next;
    }

    free_users(users);
    return 0;
}

// URL decode is provided by utils.h

void parse_login_input(char *data, char *role, char *username, char *password) {
    char *token = strtok(data, "&");
    while (token != NULL) {
        if (strncmp(token, "role=", 5) == 0) strcpy(role, token + 5);
        else if (strncmp(token, "username=", 9) == 0) strcpy(username, token + 9);
        else if (strncmp(token, "password=", 9) == 0) strcpy(password, token + 9);
        token = strtok(NULL, "&");
    }
}

int main() {
    char input[500] = {0};
    char role[MAX] = {0}, username[MAX] = {0}, password[MAX] = {0};
    char *lenstr = getenv("CONTENT_LENGTH");



    if (!lenstr) {
        printf("Content-Type: text/html\n\n");
        printf("<html><body><h3>Error: No data received</h3></body></html>");
        return 1;
    }

    int len = atoi(lenstr);
    if (len >= sizeof(input)) len = sizeof(input) - 1;
    fread(input, 1, len, stdin);
    input[len] = '\0';

    parse_login_input(input, role, username, password);
url_decode(role);
url_decode(username);
url_decode(password);

if (authenticate(role, username, password)) {

        int session_id = secure_rand_session();
        acquire_lock();
        FILE *sf = fopen("sessions.txt", "a");
        if (sf) {
            fprintf(sf, "%d %s\n", session_id, role);
            fclose(sf);
        }
        release_lock();
        printf("Set-Cookie: session_id=%d; Path=/; HttpOnly; SameSite=Strict\n", session_id);
        printf("Content-Type: text/html\n\n");
        printf("<html><head>");
        printf("<script>");
        printf("localStorage.setItem('userRole', '%s');", strcmp(role, "admin") == 0 ? "admin" : "user");
        printf("window.location.href = '/%s';", strcmp(role, "admin") == 0 ? "admin_home.html" : "home.html");
        printf("</script>");
        printf("</head><body><h2>✅ Login successful! Redirecting...</h2></body></html>");
    } else {
        printf("Content-Type: text/html\n\n");
        printf("<html><head>");
        printf("<script src='https://cdn.jsdelivr.net/npm/sweetalert2@11'></script>");
        printf("</head><body>");
        printf("<script>");
        printf("Swal.fire({\n");
        printf("  icon: 'error',\n");
        printf("  title: 'Login Failed',\n");
        printf("  text: 'Invalid username or password!',\n");
        printf("  confirmButtonText: 'Try Again'\n");
        printf("}).then(() => {\n");
        printf("  window.location.href = '/index.html';\n");
        printf("});\n");
        printf("setTimeout(() => { window.location.href = '/index.html'; }, 3000);\n");
        printf("</script>");
        printf("</body></html>");
    }

    return 0;
}
