#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 1024

void trim_newline(char *str) {
    size_t len = strlen(str);
    if (len && str[len - 1] == '\n') str[len - 1] = '\0';
}

int main() {
    FILE *fp = fopen("C:\\xampp\\cgi-bin\\reservations.txt", "r");
    if (!fp) {
        printf("Content-Type: text/plain\n\n");
        printf("Error: Cannot open reservations.txt\n");
        return 1;
    }

    // Send headers for CSV download
    printf("Content-Type: text/csv\n");
    printf("Content-Disposition: attachment; filename=\"reservations.csv\"\n\n");

    // Write CSV header
    printf("Status,Name,Email,Guests,Date,Time\n");

    char line[MAX];
    char status[50], name[100], email[100], date[20], time[20];
    int guests;

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "---", 3) == 0) {
            // Reset fields
            strcpy(status, "");
            strcpy(name, "");
            strcpy(email, "");
            strcpy(date, "");
            strcpy(time, "");
            guests = 0;

            // Status and Name line
            if (fgets(line, sizeof(line), fp)) {
                char *sep = strstr(line, " - Name: ");
                if (sep) {
                    *sep = '\0';
                    strcpy(status, line);
                    strcpy(name, sep + 9);
                    trim_newline(name);
                }
            }

            // Email
            if (fgets(line, sizeof(line), fp)) sscanf(line, "Email: %[^\n]", email);
            // Guests
            if (fgets(line, sizeof(line), fp)) sscanf(line, "Guests: %d", &guests);
            // Date
            if (fgets(line, sizeof(line), fp)) sscanf(line, "Date: %[^\n]", date);
            // Time
            if (fgets(line, sizeof(line), fp)) sscanf(line, "Time: %[^\n]", time);

            // Output CSV row
            printf("\"%s\",\"%s\",\"%s\",%d,\"%s\",\"%s\"\n", status, name, email, guests, date, time);
        }
    }

    fclose(fp);
    return 0;
}
