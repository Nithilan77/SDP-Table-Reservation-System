#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"


#define MAX_PER_SLOT 2

typedef struct Reservation {
    char name[100];
    char email[100];
    int guests;
    char time[20];
    char status[20];
    struct Reservation *next;
} Reservation;

typedef struct DateNode {
    char date[20];
    Reservation *reservations;
    struct DateNode *next;
} DateNode;

DateNode *date_list = NULL;

DateNode* get_or_create_datenode(const char *date) {
    DateNode *curr = date_list;
    while (curr) {
        if (strcmp(curr->date, date) == 0) return curr;
        curr = curr->next;
    }
    // Not found, create new
    DateNode *newDate = (DateNode*)malloc(sizeof(DateNode));
    strcpy(newDate->date, date);
    newDate->reservations = NULL;
    newDate->next = date_list;
    date_list = newDate;
    return newDate;
}

void add_reservation_to_multilist(const char *name, const char *email, int guests, const char *date, const char *time, const char *status) {
    DateNode *dateNode = get_or_create_datenode(date);

    Reservation *newRes = (Reservation *)malloc(sizeof(Reservation));
    strcpy(newRes->name, name);
    strcpy(newRes->email, email);
    newRes->guests = guests;
    strcpy(newRes->time, time);
    strcpy(newRes->status, status);
    newRes->next = dateNode->reservations;
    dateNode->reservations = newRes;
}

void load_from_text() {
    acquire_lock();
    FILE *fp = fopen("reservations.txt", "r");
    if (!fp) {
        release_lock();
        printf("Error opening file!\n");
        return;
    }

    char line[MAX];
    char name[100], email[100], status[20], date[20], time[20];
    int guests;

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "---", 3) == 0) {  // Start of reservation block
            // Clear previous values
            memset(name, 0, sizeof(name));
            memset(email, 0, sizeof(email));
            memset(status, 0, sizeof(status));
            memset(date, 0, sizeof(date));
            memset(time, 0, sizeof(time));
            guests = 0;

            // Read the status and name line
            if (fgets(line, sizeof(line), fp)) {
                // Split the first line into status and name
                char *sep = strstr(line, " - Name: ");
                if (sep) {
                    *sep = '\0';  // End the status string
                    strcpy(status, line);  // Status is on the first line
                    strcpy(name, sep + 9); // Name starts after " - Name: "
                    name[strcspn(name, "\n")] = 0; // Remove newline
                }
            }

            // Read the Email line
            if (fgets(line, sizeof(line), fp)) {
                sscanf(line, "Email: %[^\n]", email);  // Ensure to capture email correctly
            }
            // Read the Guests line
            if (fgets(line, sizeof(line), fp)) {
                sscanf(line, "Guests: %d", &guests);
            }
            // Read the Date line
            if (fgets(line, sizeof(line), fp)) {
                sscanf(line, "Date: %[^\n]", date);
                // Trim any extra spaces or newlines
                date[strcspn(date, "\n")] = 0;  // Remove newline from date
            }
            // Read the Time line
            if (fgets(line, sizeof(line), fp)) {
                sscanf(line, "Time: %[^\n]", time);
                // Trim any extra spaces or newlines
                time[strcspn(time, "\n")] = 0;  // Remove newline from time
            }

            // Debug: Print all parsed values to ensure correct data is read
            printf("Parsed: Name: %s, Email: %s, Guests: %d, Date: %s, Time: %s, Status: %s\n", 
                    name, email, guests, date, time, status);

            // Add reservation to the multilist
            add_reservation_to_multilist(name, email, guests, date, time, status);
        }
    }

    fclose(fp);
    release_lock();
}

void print_reservation_json(const Reservation *res, const char *date) {
    char clean_name[MAX*2] = {0}, clean_email[MAX*2] = {0}, clean_time[MAX*2] = {0}, clean_status[MAX*2] = {0};
    html_escape(res->name, clean_name, sizeof(clean_name));
    html_escape(res->email, clean_email, sizeof(clean_email));
    html_escape(res->time, clean_time, sizeof(clean_time));
    html_escape(res->status, clean_status, sizeof(clean_status));

    printf("  {\n");
    printf("    \"name\": \"%s\",\n", clean_name);
    printf("    \"email\": \"%s\",\n", clean_email);
    printf("    \"guests\": \"%d\",\n", res->guests);
    printf("    \"date\": \"%s\",\n", date);
    printf("    \"time\": \"%s\",\n", clean_time);
    printf("    \"status\": \"%s\"\n", clean_status);
    printf("  }");
}

void handle_reservations() {
    load_from_text();

    printf("Content-Type: application/json\n\n");
    printf("[\n");

    int first = 1;
    DateNode *currDate = date_list;
    while (currDate) {
        Reservation *res = currDate->reservations;
        while (res) {
            if (!first) printf(",\n");
            print_reservation_json(res, currDate->date);
            first = 0;
            res = res->next;
        }
        currDate = currDate->next;
    }

    printf("\n]\n");
}

// verify_session is provided by utils.h

int main() {
    if (!verify_session()) {
        printf("Status: 403 Forbidden\n");
        printf("Content-Type: application/json\n\n");
        printf("[]\n");
        return 0;
    }
    handle_reservations();
    return 0;
}