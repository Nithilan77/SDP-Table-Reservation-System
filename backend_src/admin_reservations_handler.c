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
    struct Reservation *nextWait; 
} Reservation;

typedef struct DateNode {
    char date[20];
    Reservation *reservations;
    Reservation *waitlist_head;       
    Reservation *waitlist_tail; 
    struct DateNode *next;
} DateNode;

DateNode *date_list = NULL;

DateNode* get_or_create_datenode(const char *date) {
    DateNode *curr = date_list;
    while (curr) {
        if (strcmp(curr->date, date) == 0) return curr;
        curr = curr->next;
    }
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
    newRes->next = NULL;
    newRes->nextWait = NULL;

    if (strcmp(status, "CONFIRMED") == 0) {
        newRes->next = dateNode->reservations;
        dateNode->reservations = newRes;
    } else {  // WAITLISTED
        if (!dateNode->waitlist_head) {
            dateNode->waitlist_head = dateNode->waitlist_tail = newRes;
        } else {
            dateNode->waitlist_tail->nextWait = newRes;
            dateNode->waitlist_tail = newRes;
        }
    }
}

// url_decode and parse_input are provided by utils.h

int count_existing(const char *date, const char *time) {
    int count = 0;
    char input_hour[3];
    strncpy(input_hour, time, 2);
    input_hour[2] = '\0';

    DateNode *currDate = date_list;
    while (currDate) {
        if (strcmp(currDate->date, date) == 0) {
            Reservation *res = currDate->reservations;
            while (res) {
                char res_hour[3];
                strncpy(res_hour, res->time, 2);
                res_hour[2] = '\0';

                if (strcmp(res_hour, input_hour) == 0 && strcmp(res->status, "CONFIRMED") == 0)
                    count++;
                res = res->next;
            }
            break;
        }
        currDate = currDate->next;
    }
    return count;
}

void save_to_text() {
    FILE *fp = fopen("reservations.txt", "w");
    if (!fp) {
        return;
    }

    DateNode *currDate = date_list;
    while (currDate) {
        // Save CONFIRMED reservations
        Reservation *res = currDate->reservations;
        while (res) {
            fprintf(fp, "---\n%s - Name: %s\nEmail: %s\nGuests: %d\nDate: %s\nTime: %s\n",
                    res->status, res->name, res->email, res->guests, currDate->date, res->time);
            res = res->next;
        }

        // Save WAITLISTED reservations
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
                    name[strcspn(name, "\n")] = 0;
                }
            }

            if (fgets(line, sizeof(line), fp)) {
                sscanf(line, "Email: %[^\n]", email);
            }

            if (fgets(line, sizeof(line), fp)) {
                sscanf(line, "Guests: %d", &guests);
            }

            if (fgets(line, sizeof(line), fp)) {
                sscanf(line, "Date: %[^\n]", date);
            }

            if (fgets(line, sizeof(line), fp)) {
                sscanf(line, "Time: %[^\n]", time);
            }

            add_reservation_to_multilist(name, email, guests, date, time, status);
        }
    }

    fclose(fp);
}

// verify_session is provided by utils.h

int main() {
    if (!verify_session()) {
        printf("Status: 403 Forbidden\n");
        printf("Content-Type: text/html\n\n");
        printf("<html><script>alert('Unauthorized'); window.location.href='/index.html';</script></html>");
        return 0;
    }

    char input[MAX] = {0};
    char name[100], email[100], guests[10], date[20], time[20];
    char *lenstr = getenv("CONTENT_LENGTH");

    printf("Content-Type: text/html\n\n");

    if (!lenstr) {
        printf("<html><body><script>alert('Error: No data received'); window.location.href='/admin_reservations.html';</script></body></html>");
        return 1;
    }

    int len = atoi(lenstr);
    if (len >= MAX) len = MAX - 1;
    fread(input, 1, len, stdin);
    input[len] = '\0';

    parse_input(input, name, email, guests, date, time);
    url_decode(name); url_decode(email); url_decode(guests);
    url_decode(date); url_decode(time);

    // --- ATOMIC TRANSACTION BLOCK ---
    acquire_lock();
    load_from_text();

    int guest_count = atoi(guests);
    int confirmed_count = count_existing(date, time);
    const char *status = (confirmed_count < MAX_PER_SLOT) ? "CONFIRMED" : "WAITLISTED";

    add_reservation_to_multilist(name, email, guest_count, date, time, status);
    save_to_text();
    release_lock();
    // --------------------------------

    printf("<html><head>");
    printf("<script src='https://cdn.jsdelivr.net/npm/sweetalert2@11'></script>");
    printf("</head><body>");
    printf("<script>");
    if (strcmp(status, "CONFIRMED") == 0) {
        printf("Swal.fire({icon: 'success', title: 'Reservation Confirmed!', text: 'Your table has been booked.', confirmButtonColor: '#3085d6'}).then(function(){ window.location.href='/admin_reservations.html'; });");
    } else {
        printf("Swal.fire({icon: 'info', title: 'Added to Waitlist', text: 'All tables full. You are waitlisted.', confirmButtonColor: '#3085d6'}).then(function(){ window.location.href='/admin_reservations.html'; });");
    }
    printf("</script>");
    printf("</body></html>");

    return 0;
}
