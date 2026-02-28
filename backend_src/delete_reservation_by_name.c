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
    struct Reservation *next;       // Confirmed
    struct Reservation *nextWait;   // Waitlist
} Reservation;

typedef struct DateNode {
    char date[20];
    Reservation *reservations;      // Confirmed list
    Reservation *waitlist_head;     // Waitlist queue
    Reservation *waitlist_tail;
    struct DateNode *next;
} DateNode;

DateNode *date_list = NULL;

void trim_newline(char *str) {
    size_t len = strlen(str);
    if (len && str[len - 1] == '\n') str[len - 1] = '\0';
}

DateNode* get_or_create_datenode(const char *date) {
    DateNode *curr = date_list;
    while (curr) {
        if (strcmp(curr->date, date) == 0) return curr;
        curr = curr->next;
    }
    DateNode *newDate = (DateNode*)malloc(sizeof(DateNode));
    strcpy(newDate->date, date);
    newDate->reservations = NULL;
    newDate->waitlist_head = newDate->waitlist_tail = NULL;
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
    newRes->next = NULL;
    newRes->nextWait = NULL;

    if (strcmp(status, "CONFIRMED") == 0) {
        newRes->next = dateNode->reservations;
        dateNode->reservations = newRes;
    } else {
        if (!dateNode->waitlist_head) {
            dateNode->waitlist_head = dateNode->waitlist_tail = newRes;
        } else {
            dateNode->waitlist_tail->nextWait = newRes;
            dateNode->waitlist_tail = newRes;
        }
    }
}

void load_from_text() {
    FILE *fp = fopen("reservations.txt", "r");
    if (!fp) {
        return;
    }

    char line[MAX];
    char name[100], email[100], status[20], date[20], time[20];
    int guests;

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "---", 3) == 0) {
            memset(name, 0, sizeof(name));
            memset(email, 0, sizeof(email));
            memset(status, 0, sizeof(status));
            memset(date, 0, sizeof(date));
            memset(time, 0, sizeof(time));
            guests = 0;

            if (fgets(line, sizeof(line), fp)) {
                char *sep = strstr(line, " - Name: ");
                if (sep) {
                    *sep = '\0';
                    strcpy(status, line);
                    strcpy(name, sep + 9);
                    trim_newline(name);
                }
            }
            if (fgets(line, sizeof(line), fp)) sscanf(line, "Email: %[^\n]", email);
            if (fgets(line, sizeof(line), fp)) sscanf(line, "Guests: %d", &guests);
            if (fgets(line, sizeof(line), fp)) sscanf(line, "Date: %[^\n]", date);
            if (fgets(line, sizeof(line), fp)) sscanf(line, "Time: %[^\n]", time);

            add_reservation_to_multilist(name, email, guests, date, time, status);
        }
    }

    fclose(fp);
}

void save_to_text() {
    FILE *fp = fopen("reservations.txt", "w");
    if (!fp) {
        return;
    }

    DateNode *currDate = date_list;
    while (currDate) {
        // Save confirmed
        Reservation *res = currDate->reservations;
        while (res) {
            fprintf(fp, "---\n%s - Name: %s\nEmail: %s\nGuests: %d\nDate: %s\nTime: %s\n",
                    res->status, res->name, res->email, res->guests, currDate->date, res->time);
            res = res->next;
        }
        // Save waitlist
        Reservation *wait = currDate->waitlist_head;
        while (wait) {
            fprintf(fp, "---\n%s - Name: %s\nEmail: %s\nGuests: %d\nDate: %s\nTime: %s\n",
                    wait->status, wait->name, wait->email, wait->guests, currDate->date, wait->time);
            wait = wait->nextWait;
        }

        currDate = currDate->next;
    }

    fclose(fp);
}

void get_name(char *name) {
    char *query = getenv("QUERY_STRING");
    name[0] = '\0';
    if (query) {
        sscanf(query, "name=%99[^&]", name);
        for (int i = 0; name[i]; i++) {
            if (name[i] == '+') name[i] = ' ';
        }
    }
}

void promote_waitlisted(DateNode *dateNode, const char *time) {
    Reservation *prev = NULL;
    Reservation *curr = dateNode->waitlist_head;
    while (curr) {
        if (strcmp(curr->time, time) == 0) {
            // Promote
            strcpy(curr->status, "CONFIRMED");

            // Remove from waitlist queue
            if (prev == NULL) {
                dateNode->waitlist_head = curr->nextWait;
                if (dateNode->waitlist_tail == curr) {
                    dateNode->waitlist_tail = NULL;
                }
            } else {
                prev->nextWait = curr->nextWait;
                if (dateNode->waitlist_tail == curr) {
                    dateNode->waitlist_tail = prev;
                }
            }

            // Add to front of confirmed list
            curr->next = dateNode->reservations;
            dateNode->reservations = curr;

            break; // Promote only one
        }
        prev = curr;
        curr = curr->nextWait;
    }
}

// verify_session is provided by utils.h

int main() {
    if (!verify_session()) {
        printf("Status: 403 Forbidden\nContent-Type: text/plain\n\nError: Unauthorized\n");
        return 0;
    }
    char nameToDelete[100];
    get_name(nameToDelete);

    printf("Content-Type: text/plain\n\n");

    if (strlen(nameToDelete) == 0) {
        printf("No name provided.");
        return 0;
    }

    // ATOMIC BLOCK
    acquire_lock();
    load_from_text();

    int deleted = 0;
    DateNode *dptr = date_list;
    while (dptr) {
        Reservation *prev = NULL;
        Reservation *curr = dptr->reservations;

        while (curr) {
            if (strcasecmp(curr->name, nameToDelete) == 0) {
                char deleted_time[20];
                strcpy(deleted_time, curr->time);

                if (prev == NULL)
                    dptr->reservations = curr->next;
                else
                    prev->next = curr->next;

                free(curr);
                deleted = 1;

                // Promote from waitlist
                promote_waitlisted(dptr, deleted_time);
                break;
            }
            prev = curr;
            curr = curr->next;
        }

        if (deleted) break;
        dptr = dptr->next;
    }

    save_to_text();
    release_lock();
    // --------------------------------

    if (deleted)
        printf("Deleted reservation for '%s' and promoted from waitlist if any.\n", nameToDelete);
    else
        printf("No reservation found for '%s'.\n", nameToDelete);

    return 0;
}
