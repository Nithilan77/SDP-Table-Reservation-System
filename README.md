# 🍽️ MR Pandian Hotel - Table Reservation System

<p align="center">
  <img src="https://img.shields.io/badge/Backend-C%20(C99)-blue.svg" alt="C Language">
  <img src="https://img.shields.io/badge/Server-Apache%20%2F%20XAMPP-orange.svg" alt="XAMPP">
  <img src="https://img.shields.io/badge/Frontend-HTML%20%7C%20CSS%20%7C%20JS-yellow.svg" alt="Frontend">
  <img src="https://img.shields.io/badge/Storage-Flat--File%20(Mutex%20Locked)-lightgrey.svg" alt="Storage">
</p>

## Overview
A low-level, high-performance web application designed to handle restaurant table bookings and queueing logistics. Entirely bypassing modern heavy web frameworks (like Django or Express), this system's backend is engineered from scratch in **pure C**, compiled into CGI executables. 

It natively intercepts raw HTTP protocols, dynamically ingests payloads, and securely manipulates text-based databases under intense concurrent traffic conditions using native Windows OS Mutex locks.

## ✨ Core Features
*   **C-Native Backend Engine:** Lightning-fast CGI execution handling HTTP `GET`/`POST` parsing exclusively in C.
*   **Atomic Multithreaded Database:** Utilizes `<windows.h>` `CreateMutex()` hooks to safely lock concurrent transactions, preventing the Time-Of-Check to Time-Of-Use (TOCTOU) race conditions common in file-based storage.
*   **Dynamic Waitlist Algorithm:** Employs a chronologically nested linked-list architecture in C memory to dynamically demote overflowing reservations to a queue and automatically promote them upon cancellations.
*   **Role-Based Access Control (RBAC):** Distinct User and Admin dashboards protected natively by cryptographic hashing and `HttpOnly` Secure Session IDs.
*   **Admin Utilities:** Direct database interfacing allowing for dynamic reservation edits, Name-based record expunging, and one-click `.CSV` exporting.

## 🛡️ Security Engineering
This system has undergone extensive, production-grade security sweeps to ensure absolute memory safety and architectural integrity:

*   **Stack Buffer Overflow Protection:** Zero usage of unbounded `gets()` or raw `strcpy()`. All memory ingestion (`fread`, `strncpy`) explicitly bounds to `sizeof(buffer) - 1`.
*   **Memory Exhaustion (DoS) Defense:** The registration and reservation handlers algorithmically reject `malloc()` requests exceeding 10KB.
*   **Strict XSS Sanitization:** A custom `html_escape()` C-serializer captures and converts raw `<script>` tags before returning dynamic HTML to the client browser.
*   **Insecure Direct Object Reference (IDOR) Protection:** Every restricted endpoint is hard-gated by a `verify_session()` cookie tracker.

## 🚀 Installation & Deployment (Windows)

### 1. Prerequisites
*   [XAMPP](https://www.apachefriends.org/index.html) installed locally.
*   `MinGW-w64` (GCC Compiler) added to your Environment `PATH`.

### 2. Frontend Configuration
Copy the entire `frontend` directory into XAMPP's public serving folder:
```bash
xcopy /E /I "frontend" "C:\xampp\htdocs\mr_pandian"
```

### 3. Backend Compilation (CGI)
Navigate into the `backend_src` directory and utilize GCC to compile the raw `C` source code into `.cgi` executable binaries, placing them directly in the Apache execution folder:
```bash
cd backend_src

# Compile the shared utility engine
gcc -c utils.c -o utils.o

# Compile the handler blocks (Example)
gcc reservation_handler.c utils.o -o C:\xampp\cgi-bin\reservation_handler.cgi
gcc index_handler.c utils.o -o C:\xampp\cgi-bin\index_handler.cgi
gcc admin_dashboard_handler.c utils.o -o C:\xampp\cgi-bin\admin_dashboard_handler.cgi
# Repeat for all remaining handlers...
```

### 4. Database Initialization
Ensure the text databases are located exactly where your handlers expect them (usually mirroring the executable path or hard-coded).

### 5. Launch
Start the **Apache** service inside the XAMPP Control Panel.
Navigate to: `http://localhost/mr_pandian/index.html`

## 👨‍💻 Author
Designed and Architected by **Nithilan** (3rd-Semester IT Student).
