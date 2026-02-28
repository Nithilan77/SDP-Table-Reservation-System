#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"

#define MENU_FILE "menu.txt"
#define RESERVATION_FILE "reservations.txt"
#define MESSAGE_FILE "messages.txt"

#define ADMIN_ID "admin"
#define ADMIN_PASS "admin123"
#define USER_ID "user"
#define USER_PASS "user123"

// Structure for table reservation
typedef struct Reservation {
    char customerName[50];
    char email[50];
    int numGuests;
    char date[15];
    char time[10];
    struct Reservation *next;
} Reservation;

Reservation *head = NULL;

// Multilist node for each date
typedef struct ResNode {
    char customerName[50];
    char email[50];
    int numGuests;
    char time[10];
    struct ResNode *nextRes;
} ResNode;

typedef struct DateNode {
    char date[15];
    ResNode *reservations;
    struct DateNode *nextDate;
} DateNode;

DateNode *dateHead = NULL;

// Queue for waitlist
typedef struct QueueNode {
    char name[50];
    char email[50];
    int guests;
    char date[15];
    char time[10];
    struct QueueNode *next;
} QueueNode;

QueueNode *front = NULL, *rear = NULL;

// Function to validate email format
int isValidEmail(char *email) {
    if (!(strchr(email, '@') && strchr(email, '.'))) {
        printf("==>Error: Invalid email format. Please enter a valid email.\n");
        return 0;
    }
    return 1;
}

// Function to validate date format (DD/MM/YYYY)
int isValidDate(char *date) {
    if (strlen(date) != 10 || date[2] != '/' || date[5] != '/') {
        printf("==>Error: Invalid date format. Use DD/MM/YYYY.\n");
        return 0;
    }
    for (int i = 0; i < 10; i++) {
        if (i == 2 || i == 5) continue;
        if (!isdigit(date[i])) {
            printf("==>Error: Date must contain only numbers.\n");
            return 0;
        }
    }
    return 1;
}

// Function to validate time format (HH:MM)
int isValidTime(char *time) {
    if (strlen(time) != 5 || time[2] != ':') {
        printf("==>Error: Invalid time format. Use HH:MM.\n");
        return 0;
    }
    for (int i = 0; i < 5; i++) {
        if (i == 2) continue;
        if (!isdigit(time[i])) {
            printf("==>Error: Time must contain only numbers.\n");
            return 0;
        }
    }
    if (!(strcmp(time, "10:00") >= 0 && strcmp(time, "22:00") <= 0)) {
        printf("==>Error: Time must be between 10:00 and 22:00.\n");
        return 0;
    }
    return 1;
}

// Function to insert reservation into multilist (grouped by date)
void insertIntoMultilist(char *name, char *email, int guests, char *date, char *time) {
    DateNode *dptr = dateHead, *prev = NULL;

    while (dptr != NULL && strcmp(dptr->date, date) != 0) {
        prev = dptr;
        dptr = dptr->nextDate;
    }

    if (dptr == NULL) {
        dptr = (DateNode *)malloc(sizeof(DateNode));
        strcpy(dptr->date, date);
        dptr->reservations = NULL;
        dptr->nextDate = NULL;

        if (prev == NULL) {
            dateHead = dptr;
        } else {
            prev->nextDate = dptr;
        }
    }

    ResNode *r = (ResNode *)malloc(sizeof(ResNode));
    strcpy(r->customerName, name);
    strcpy(r->email, email);
    r->numGuests = guests;
    strcpy(r->time, time);
    r->nextRes = dptr->reservations;
    dptr->reservations = r;
}

// Function to add reservation
void addReservation(char *name, char *email, int guests, char *date, char *time) {
    if (!isValidEmail(email)) {
        printf("Invalid email format. Please try again.\n");
        return;
    }
    if (!isValidDate(date)) {
        printf("Invalid date format. Use DD/MM/YYYY.\n");
        return;
    }
    if (!isValidTime(time)) {
        printf("Invalid time. Use HH:MM between 10:00 and 22:00.\n");
        return;
    }
    if (guests <= 0) {
        printf("Number of guests must be positive.\n");
        return;
    }

    // Add to multilist (group by date)
    insertIntoMultilist(name, email, guests, date, time);

    acquire_lock();
    FILE *file = fopen(RESERVATION_FILE, "a");
    if(file) {
        fprintf(file, "---\nCONFIRMED - Name: %s\nEmail: %s\nGuests: %d\nDate: %s\nTime: %s\n", name, email, guests, date, time);
        fclose(file);
    }
    release_lock();
}

// Function to display reservations in a properly aligned table
void displayReservations() {
    acquire_lock();
    FILE *file = fopen(RESERVATION_FILE, "r");
    char name[50], email[50], date[15], time[10], status[20];
    int guests;
    char line[256];

    if (file == NULL) {
        release_lock();
        printf("No reservations found.\n");
        return;
    }
    printf("\n------------------------------------------------------------------------------------------------------\n");
    printf("| %-15s | %-20s | %-25s | %-6s | %-12s | %-5s |\n", "Status", "Name", "Email", "Guests", "Date", "Time");
    printf("------------------------------------------------------------------------------------------------------\n");
    
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "---", 3) == 0) {
            memset(name, 0, sizeof(name));
            memset(email, 0, sizeof(email));
            memset(date, 0, sizeof(date));
            memset(time, 0, sizeof(time));
            memset(status, 0, sizeof(status));
            guests = 0;

            if (fgets(line, sizeof(line), file)) {
                char *sep = strstr(line, " - Name: ");
                if (sep) {
                    *sep = '\0';
                    strcpy(status, line);
                    strcpy(name, sep + 9);
                    name[strcspn(name, "\n")] = 0;
                }
            }

            if (fgets(line, sizeof(line), file)) {
                sscanf(line, "Email: %[^\n]", email);
            }
            if (fgets(line, sizeof(line), file)) {
                sscanf(line, "Guests: %d", &guests);
            }
            if (fgets(line, sizeof(line), file)) {
                sscanf(line, "Date: %[^\n]", date);
            }
            if (fgets(line, sizeof(line), file)) {
                sscanf(line, "Time: %[^\n]", time);
            }

            printf("| %-15s | %-20s | %-25s | %-6d | %-12s | %-s |\n", status, name, email, guests, date, time);
        }
    }
    printf("------------------------------------------------------------------------------------------------------\n");
    fclose(file);
    release_lock();
}

// Function to cancel a reservation
void cancelReservation() {
    acquire_lock();
    FILE *file = fopen(RESERVATION_FILE, "r");
    if (!file) {
        release_lock();
        printf("No reservations found.\n");
        return;
    }

    FILE *tempFile = fopen("temp.txt", "w");
    char name[50], date[15], time[10], email[50], resName[50], resDate[15], resTime[10], status[20];
    int guests, found = 0;
    char line[256];

    printf("Enter your name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;
    printf("Enter reservation date (DD/MM/YYYY): ");
    fgets(date, sizeof(date), stdin);
    date[strcspn(date, "\n")] = 0;

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "---", 3) == 0) {
            memset(resName, 0, sizeof(resName));
            memset(email, 0, sizeof(email));
            memset(resDate, 0, sizeof(resDate));
            memset(resTime, 0, sizeof(resTime));
            memset(status, 0, sizeof(status));
            guests = 0;

            if (fgets(line, sizeof(line), file)) {
                char *sep = strstr(line, " - Name: ");
                if (sep) {
                    *sep = '\0';
                    strcpy(status, line);
                    strcpy(resName, sep + 9);
                    resName[strcspn(resName, "\n")] = 0;
                }
            }

            if (fgets(line, sizeof(line), file)) {
                sscanf(line, "Email: %[^\n]", email);
            }
            if (fgets(line, sizeof(line), file)) {
                sscanf(line, "Guests: %d", &guests);
            }
            if (fgets(line, sizeof(line), file)) {
                sscanf(line, "Date: %[^\n]", resDate);
            }
            if (fgets(line, sizeof(line), file)) {
                sscanf(line, "Time: %[^\n]", resTime);
            }

            if (strcmp(name, resName) == 0 && strcmp(date, resDate) == 0) {
                found = 1;
                continue;
            }
            
            fprintf(tempFile, "---\n%s - Name: %s\nEmail: %s\nGuests: %d\nDate: %s\nTime: %s\n", status, resName, email, guests, resDate, resTime);
        }
    }
    fclose(file);
    fclose(tempFile);

    remove(RESERVATION_FILE);
    rename("temp.txt", RESERVATION_FILE);

    if (found) {
        printf("Reservation cancelled successfully.\n");
    } else {
        printf("Reservation not found.\n");
    }
    release_lock();
}

// Function to view menu
void viewMenu() {
    acquire_lock();
    FILE *file = fopen("menu.txt", "r");
    if (!file) {
        release_lock();
        printf("NO ITEMS FOUND\n");
        return;
    }

    printf("\n-----------------------------------------\n");
    printf("| %-20s | %-10s |\n", "Menu Items", "Price");
    printf("-----------------------------------------\n");

    char line[100], item[50];
    float price;
    int found = 0;

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';

        char *lastSpace = strrchr(line, ' ');
        if (lastSpace) {
            *lastSpace = '\0';
            sscanf(lastSpace + 1, "%f", &price);
            strcpy(item, line);
        } else {
            continue;
        }

        printf("| %-20s | %-10.2f |\n", item, price);
        found = 1;
    }

    if (!found) {
        printf("NO ITEMS FOUND\n");
    }

    printf("-----------------------------------------\n");
    fclose(file);
    release_lock();
}

// Function to modify menu
void modifyMenu() {
    acquire_lock();
    FILE *file = fopen(MENU_FILE, "w");
    if (!file) {
        release_lock();
        printf("Error opening menu file!\n");
        return;
    }
    char item[50];
    char line[50];
    float price;
    printf("Enter new menu items with prices (type END to stop):\n");
    while (1) {
        printf("Item Name: ");
        fgets(item, sizeof(item), stdin);
        item[strcspn(item, "\n")] = '\0';
        if (strcmp(item, "END") == 0) break;
        printf("Price: ");
        fgets(line, sizeof(line), stdin);
        sscanf(line, "%f", &price);
        fprintf(file, "%s %.2f\n", item, price);
    }
    fclose(file);
    release_lock();
    printf("Menu updated successfully!\n");
}

// Function to authenticate user/admin
int authenticate(char role) {
    char id[20], pass[20];
    printf("Enter ID: ");
    scanf("%19s", id);
    printf("Enter Password: ");
    scanf("%19s", pass);

    if ((role == 'A' && strcmp(id, ADMIN_ID) == 0 && strcmp(pass, ADMIN_PASS) == 0) ||
        (role == 'U' && strcmp(id, USER_ID) == 0 && strcmp(pass, USER_PASS) == 0)) {
        return 1;
    }

    printf("Invalid login credentials.\n");
    return 0;
}

// Function to send a message to admin
void sendMessage() {
    char name[50], message[200];
    printf("Enter your name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;
    printf("Enter your message to admin: ");
    fgets(message, sizeof(message), stdin);

    acquire_lock();
    FILE *file = fopen(MESSAGE_FILE, "a");
    if(file) {
        fprintf(file, "Message from %s: %s", name, message);
        fclose(file);
    }
    release_lock();
    
    printf("Message sent successfully!\n");
}

int main() {
    int choice, authenticated = 0, isAdmin = 0;
    char role, loginAgain;

    do {
        printf("Welcome to the Hotel Management System!\n");
        printf("Enter 'A' for Admin or 'U' for User: ");
        scanf(" %c", &role);

        if (role == 'A') {
            authenticated = authenticate('A');
            if (authenticated) isAdmin = 1;
        } else if (role == 'U') {
            authenticated = authenticate('U');
        } else {
            printf("Invalid role! Exiting...\n");
            return 0;
        }

        if (!authenticated) return 0;

        do {
            // Admin menu
            if (isAdmin) {
                printf("\nADMIN MENU\n");
                printf("1. View Reservations\n");
                printf("2. Modify Menu\n");
                printf("3. View Messages\n");
                printf("4. Exit\n");
                printf("Enter your choice: ");
                scanf("%d", &choice);

                switch (choice) {
                    case 1:
                        displayReservations();
                        break;
                    case 2:
                        modifyMenu();
                        break;
                    case 3:
                        printf("Messages feature is under development!\n");
                        break;
                    case 4:
                        printf("Exiting admin menu.\n");
                        break;
                    default:
                        printf("Invalid choice. Try again.\n");
                }
            } else {
                // User menu
                printf("\nUSER MENU\n");
                printf("1. Add Reservation\n");
                printf("2. Cancel Reservation\n");
                printf("3. View Menu\n");
                printf("4. Send Message to Admin\n");
                printf("5. View Waitlist\n");
                printf("6. Exit\n");
                printf("Enter your choice: ");
                scanf("%d", &choice);

                switch (choice) {
                    case 1: {
                        char name[50], email[50], date[15], time[10];
                        int guests;
                        printf("Enter your name: ");
                        scanf(" %49[^\n]", name);
                        printf("Enter your email: ");
                        scanf(" %49s", email);
                        printf("Enter number of guests: ");
                        scanf("%d", &guests);
                        printf("Enter reservation date (DD/MM/YYYY): ");
                        scanf(" %14s", date);
                        printf("Enter reservation time (HH:MM): ");
                        scanf(" %9s", time);

                        addReservation(name, email, guests, date, time);
                        break;
                    }
                    case 2:
                        cancelReservation();
                        break;
                    case 3:
                        viewMenu();
                        break;
                    case 4:
                        sendMessage();
                        break;
                    case 5:
                        printf("Waitlist feature under development.\n");
                        break;
                    case 6:
                        printf("Exiting user menu.\n");
                        break;
                    default:
                        printf("Invalid choice. Try again.\n");
                }
            }
        } while (choice != (isAdmin ? 4 : 6));

        printf("Do you wish to log in again? (Y/N): ");
        scanf(" %c", &loginAgain);
    } while (loginAgain == 'Y' || loginAgain == 'y');

    printf("Thank you for using the Hotel Management System!\n");

    return 0;
}

