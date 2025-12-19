# SDP - Table Reservation System

A Hotel Table Reservation System featuring both a **Console-based Application** and a **Web-based CGI Application**. This system allows users to book tables, check availability, view menus, and contact administration, while giving Key Administrators tools to manage reservations and the menu.

## Features

### User Features
- **Table Reservation:** Book tables by providing details (Name, Email, Guests, Date, Time).
- **Waitlist System:** Automatically adds users to a waitlist if the desired time slot is fully booked.
- **Menu Viewing:** Browse the restaurant menu with prices.
- **Contact Admin:** Send messages or queries to the restaurant administration.
- **Validation:** Input validation for emails, dates, and time slots.

### Admin Features
- **Dashboard:** Access via secure login (default credentials below).
- **Manage Reservations:** View and manage all confirmed and waitlisted reservations.
- **Menu Management:** Add, update, or remove items from the menu.
- **View Messages:** Read messages sent by customers.

## Technologies Used
- **Backend:** C Language
- **Frontend:** HTML, CSS, JavaScript (SweetAlert2 for notifications)
- **Server-Side Scripting:** CGI (Common Gateway Interface) with C
- **Data Persistence:** Text-based file storage (`reservations.txt`, `menu.txt`, etc.)

## Project Structure
The project contains two versions of the application:
1.  **Console Version:** `Table Reservation - backend.c` (Standalone executable).
2.  **Web Version:** Collection of `.html` files and `.c` handlers (compiled to `.cgi` or `.exe` for CGI bin).

## Setup & Installation

### Prerequisites
- GCC Compiler (MinGW for Windows)
- XAMPP (or any Web Server with CGI support) - *Recommended for Web Version*

### 1. Web Version Setup (XAMPP)
**Note:** The source code currently contains hardcoded paths pointing to `C:\xampp\cgi-bin\`. If your XAMPP installation is different, you must update the paths in the `.c` files and recompile.

1.  **Install XAMPP** and ensure Apache is running.
2.  **Frontend Files:** Move all `.html`, `.css`, `.js`, and image files (`.jpg`) to the `C:\xampp\htdocs\` directory (or a subdirectory inside it).
3.  **Backend Files:**
    -   Compile the `.c` handler files (`reservation_handler.c`, `admin_dashboard_handler.c`, etc.) to executables.
    -   Move the compiled executables (`.exe`) to `C:\xampp\cgi-bin\`.
    -   Ensure the data files (`reservations.txt`, `menu.txt`, `messages.txt`) are located in `C:\xampp\cgi-bin\` (or the path specified in the source code).

#### Compilation Example
```bash
gcc reservation_handler.c -o reservation_handler.exe
gcc admin_dashboard_handler.c -o admin_dashboard_handler.exe
# Repeat for other handlers
```

### 2. Console Version Setup
1.  Open a terminal.
2.  Compile the backend logic:
    ```bash
    gcc "Table Reservation - backend.c" -o table_reservation.exe
    ```
3.  Run the executable:
    ```bash
    ./table_reservation.exe
    ```

## Usage

### Default Credentials
- **Admin ID:** `admin`
- **Password:** `admin123`
- **User ID:** `user`
- **Password:** `user123`

### Web Interface
1.  Open your browser and navigate to `http://localhost/index.html` (or your specific folder path).
2.  Use the navigation links to Reserve a Table, View Menu, or Login as Admin.

## Troubleshooting
- **500 Server Error:** Usually indicates a CGI error. Check that the `.exe` files are in `cgi-bin` and have execution permissions.
- **File Not Found:** Ensure the text files (`reservations.txt`, etc.) exist in the directory expected by the C program.
