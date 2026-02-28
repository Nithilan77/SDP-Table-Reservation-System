#include "utils.h"

HANDLE hMutex = NULL;

void acquire_lock() {
    hMutex = CreateMutex(NULL, FALSE, "Global\\MR_PANDIAN_RESERVATION_MUTEX");
    if (hMutex != NULL) {
        WaitForSingleObject(hMutex, INFINITE);
    }
}

void release_lock() {
    if (hMutex != NULL) {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        hMutex = NULL;
    }
}

void url_decode(char *src) {
    char *p = src;
    char code[3] = {0};
    while (*src) {
        if (*src == '+') *p++ = ' ';
        else if (*src == '%' && src[1] && src[2]) {
            code[0] = src[1];
            code[1] = src[2];
            *p++ = (char)strtol(code, NULL, 16);
            src += 2;
        } else {
            *p++ = *src;
        }
        src++;
    }
    *p = '\0';
}

void html_escape(const char *src, char *dest, size_t dest_size) {
    size_t written = 0;
    while (*src && written < dest_size - 7) { 
        switch (*src) {
            case '<': strcpy(dest, "&lt;"); dest += 4; written += 4; break;
            case '>': strcpy(dest, "&gt;"); dest += 4; written += 4; break;
            case '&': strcpy(dest, "&amp;"); dest += 5; written += 5; break;
            case '"': strcpy(dest, "&quot;"); dest += 6; written += 6; break;
            case '\'': strcpy(dest, "&#39;"); dest += 5; written += 5; break;
            default: *dest++ = *src; written++; break;
        }
        src++;
    }
    *dest = '\0';
}

unsigned long simple_hash(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

int secure_rand_session() {
    // Generate a secure pseudo-random session ID
    srand((unsigned int)(time(NULL) ^ GetCurrentProcessId() ^ GetCurrentThreadId()));
    return rand();
}

int verify_session() {
    char *cookie = getenv("HTTP_COOKIE");
    if (!cookie) return 0;
    
    char *session_str = strstr(cookie, "session_id=");
    if (!session_str) return 0;
    
    int session_id;
    if (sscanf(session_str, "session_id=%d", &session_id) != 1) return 0;

    int valid = 0;
    acquire_lock(); // Prevent race conditions reading sessions
    FILE *sf = fopen("sessions.txt", "r");
    if (sf) {
        int s_id;
        char s_role[20];
        while (fscanf(sf, "%d %19s", &s_id, s_role) == 2) {
            if (s_id == session_id && strcmp(s_role, "admin") == 0) {
                valid = 1;
                break;
            }
        }
        fclose(sf);
    }
    release_lock();
    
    return valid;
}

void parse_input(char *data, char *name, char *email, char *guests, char *date, char *time) {
    // Clear buffers
    if(name) name[0] = '\0';
    if(email) email[0] = '\0';
    if(guests) guests[0] = '\0';
    if(date) date[0] = '\0';
    if(time) time[0] = '\0';

    char *token = strtok(data, "&");
    while (token) {
        if (name && strncmp(token, "name=", 5) == 0) strncpy(name, token + 5, 99);
        else if (email && strncmp(token, "email=", 6) == 0) strncpy(email, token + 6, 99);
        else if (guests && strncmp(token, "guests=", 7) == 0) strncpy(guests, token + 7, 9);
        else if (date && strncmp(token, "date=", 5) == 0) strncpy(date, token + 5, 19);
        else if (time && strncmp(token, "time=", 5) == 0) strncpy(time, token + 5, 19);
        token = strtok(NULL, "&");
    }
}
