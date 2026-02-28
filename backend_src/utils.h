#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>

#define MAX 1024

// Mutex Locking Functions to prevent concurrent file corruption
void acquire_lock();
void release_lock();

// Input Parsing & Sanitization
void url_decode(char *src);
void html_escape(const char *src, char *dest, size_t dest_size);
void parse_input(char *data, char *name, char *email, char *guests, char *date, char *time);

// Security & Authentication
int verify_session();
int secure_rand_session();
unsigned long simple_hash(const char *str);

#endif // UTILS_H
