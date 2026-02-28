# 🍽️ MR Pandian Hotel - Table Reservation System

<p align="center">
  <img src="https://img.shields.io/badge/Backend-C-blue.svg" alt="C Language">
  <img src="https://img.shields.io/badge/Server-Apache%20%2F%20XAMPP-orange.svg" alt="XAMPP">
  <img src="https://img.shields.io/badge/Frontend-HTML%20%7C%20CSS%20%7C%20JS-yellow.svg" alt="Frontend">
  <img src="https://img.shields.io/badge/Storage-Text%20Files-lightgrey.svg" alt="Storage">
</p>

## Overview
A fast, lightweight web application designed to handle restaurant table bookings and manage waitlists. Instead of using heavy modern frameworks, this system is built entirely from scratch using the **C programming language** for maximum speed and efficiency.

The system handles everything from receiving bookings from the website, securely storing them in text files, and ensuring they are managed safely even when multiple waiters try to use the system at the exact same time.

## ✨ Core Features
*   **Fast Backend Engine:** Written entirely in C, making it incredibly fast at receiving and processing form submissions.
*   **Safe Multi-User Booking:** Uses special locks ("Mutex locks") so if two people try to book a table at the exact same millisecond, the system safely processes them one by one without losing data.
*   **Smart Waitlist System:** Automatically moves overflowing reservations to a waitlist.
*   **Secure Logins:** Separate dashboards for normal Users and Restaurant Admins. Passwords are securely scrambled (hashed) so they can never be stolen.
*   **Admin Dashboard:** Allows managers to view all bookings, edit them, delete them safely, and even download them to Excel (.CSV) with one click.

## 🛡️ Security Highlights
This system was built with security in mind to protect against hackers:
*   **Crash Protection:** The system automatically rejects files or data that are too large, preventing the server from crashing.
*   **Safe Memory Handling:** Carefully limits how much memory can be used so attackers cannot overflow the system.
*   **Hack Prevention:** Automatically cleans up any dangerous code (like `<script>` tags) that someone might try to type into a reservation name.
*   **Protected Pages:** You cannot access the admin dashboard unless you are properly logged in.

## 🚀 How to Run It (For Windows)

### 1. What You Need
*   Install a free local server called [XAMPP](https://www.apachefriends.org/index.html).

### 2. Setup the Website
Copy the `frontend` folder into XAMPP's public folder:
* Place it inside: `C:\xampp\htdocs\`

### 3. Setup the Backend
Take the ready-to-use `.cgi` files (found in the `compiled_binaries` folder) and place them into the Apache execution folder:
* Place them inside: `C:\xampp\cgi-bin\`

### 4. Turn It On
1. Open the **XAMPP Control Panel**.
2. Click **Start** next to **Apache**.
3. Open your web browser and go to: `http://localhost/frontend/index.html`

## 👨‍💻 Author
Designed and Built by **Nithilan** (3rd-Semester IT Student).
