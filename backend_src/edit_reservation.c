#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"

#define MAX_LINE 256
#define MAX_RESERVATIONS 100

typedef struct {
    char status[20];
    char name[100];
    char email[100];
    int guests;
    char date[20];
    char time[20];
} Reservation;

void trim(char *str) {
    str[strcspn(str, "\r\n")] = '\0';
    int len = strlen(str);
    while (len > 0 && isspace(str[len - 1])) str[--len] = '\0';
    while (*str && isspace(*str)) memmove(str, str + 1, strlen(str));
}

void decodeURL(char *src, char *dest) {
    char a, b;
    while (*src) {
        if ((*src == '%') &&
            ((a = src[1]) && (b = src[2])) &&
            (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a') a -= 'a' - 'A';
            if (a >= 'A') a -= ('A' - 10); else a -= '0';
            if (b >= 'a') b -= 'a' - 'A';
            if (b >= 'A') b -= ('A' - 10); else b -= '0';
            *dest++ = 16 * a + b;
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

int main() {
    if (!verify_session()) {
        printf("Status: 403 Forbidden\nContent-Type: text/plain\n\nError: Unauthorized\n");
        return 0;
    }

    printf("Content-Type: text/html\n\n");

    char *query = getenv("QUERY_STRING");
    if (!query) {
        printf("Error: No query string provided.\n");
        return 1;
    }

    char name[100], newEmail[100], newGuestsStr[10], newDate[20], newTime[20];
    char raw[500];
    strncpy(raw, query, sizeof(raw) - 1);
    raw[sizeof(raw) - 1] = '\0';

    sscanf(raw, "name=%99[^&]&email=%99[^&]&guests=%9[^&]&date=%19[^&]&time=%19s",
           name, newEmail, newGuestsStr, newDate, newTime);

    decodeURL(name, name);
    decodeURL(newEmail, newEmail);
    decodeURL(newGuestsStr, newGuestsStr);
    decodeURL(newDate, newDate);
    decodeURL(newTime, newTime);

    int newGuests = atoi(newGuestsStr);

    // --- ATOMIC TRANSACTION BLOCK ---
    acquire_lock();
    FILE *fp = fopen("reservations.txt", "r");
    if (!fp) {
        release_lock();
        printf("Error opening file.\n");
        return 1;
    }

    Reservation reservations[MAX_RESERVATIONS];
    int count = 0;
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "---", 3) == 0) {
            strcpy(reservations[count].status, "CONFIRMED"); // Default
            fgets(line, sizeof(line), fp);
            sscanf(line, "%*s - Name: %[^\n]", reservations[count].name);
            fgets(line, sizeof(line), fp);
            sscanf(line, "Email: %[^\n]", reservations[count].email);
            fgets(line, sizeof(line), fp);
            sscanf(line, "Guests: %d", &reservations[count].guests);
            fgets(line, sizeof(line), fp);
            sscanf(line, "Date: %[^\n]", reservations[count].date);
            fgets(line, sizeof(line), fp);
            sscanf(line, "Time: %[^\n]", reservations[count].time);
            count++;
        }
    }
    fclose(fp);

    int found = 0;
    for (int i = 0; i < count; i++) {
        trim(reservations[i].name);
        trim(name);
        if (strcasecmp(reservations[i].name, name) == 0) {
            strcpy(reservations[i].email, newEmail);
            reservations[i].guests = newGuests;
            strcpy(reservations[i].date, newDate);
            strcpy(reservations[i].time, newTime);
            found = 1;
            break;
        }
    }

    if (!found) {
        release_lock();
        printf("No reservation with name '%s' found.\n", name);
        return 0;
    }

    // Write updated reservations back
    fp = fopen("reservations.txt", "w");
    for (int i = 0; i < count; i++) {
        fprintf(fp, "---\n");
        fprintf(fp, "CONFIRMED - Name: %s\n", reservations[i].name);
        fprintf(fp, "Email: %s\n", reservations[i].email);
        fprintf(fp, "Guests: %d\n", reservations[i].guests);
        fprintf(fp, "Date: %s\n", reservations[i].date);
        fprintf(fp, "Time: %s\n", reservations[i].time);
    }
    fclose(fp);
    release_lock();
    // --------------------------------

    printf("Reservation for '%s' updated successfully.<br>", name);
    printf("<a href='/admin_dashboard.html'>Go back to Dashboard</a>");
    return 0;
}
