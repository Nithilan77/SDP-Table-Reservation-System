#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 256

// Function to delete a reservation by its ID from the text file
int delete_reservation(int reservation_id) {
    FILE *file, *temp_file;
    char line[MAX_LINE_LENGTH];
    int id;
    int found = 0;

    // Open the original file in read mode
    file = fopen("C:\\xampp\\cgi-bin\\reservations.txt", "r");
    if (file == NULL) {
        perror("Unable to open reservations file");
        return 0;  // Failure to open file
    }

    // Create a temporary file to store data
    temp_file = fopen("C:\\xampp\\cgi-bin\\temp_reservations.txt", "w");
    if (temp_file == NULL) {
        perror("Unable to create temporary file");
        fclose(file);
        return 0;  // Failure to create temp file
    }

    // Read each line, check if it matches the reservation_id
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%d", &id);  // Get the reservation ID from the line

        // If the reservation ID matches, skip writing it to the temp file (delete it)
        if (id == reservation_id) {
            found = 1;  // Reservation found for deletion
            continue;  // Skip writing this line to the temp file
        }

        // Write the line to the temp file (preserve other reservations)
        fprintf(temp_file, "%s", line);
    }

    fclose(file);
    fclose(temp_file);

    // If reservation was not found, return 0 (failure)
    if (!found) {
        remove("C:\\xampp\\cgi-bin\\temp_reservations.txt");  // Delete the temp file if no reservation was found
        return 0;  // Reservation not found
    }

    // Remove the original file and rename the temp file to the original file name
    remove("C:\\xampp\\cgi-bin\\reservations.txt");
    rename("C:\\xampp\\cgi-bin\\temp_reservations.txt", "C:\\xampp\\cgi-bin\\reservations.txt");

    return 1;  // Successfully deleted the reservation
}

int main() {
    // Get reservation_id from query string
    char *query = getenv("QUERY_STRING");
    if (query == NULL) {
        printf("Content-Type: text/html\n\n");
        printf("<h1>Error: Missing reservation ID</h1>");
        return 0;
    }

    int reservation_id = 0;
    if (sscanf(query, "id=%d", &reservation_id) != 1) {
        printf("Content-Type: text/html\n\n");
        printf("<h1>Error: Invalid reservation ID</h1>");
        return 0;
    }

    // Perform deletion
    int result = delete_reservation(reservation_id);

    // Return a response to the client
    printf("Content-Type: application/json\n\n");
    if (result) {
        printf("{\"success\": true, \"message\": \"Reservation deleted successfully!\"}");
    } else {
        printf("{\"success\": false, \"message\": \"Failed to delete reservation.\"}");
    }

    return 0;
}
