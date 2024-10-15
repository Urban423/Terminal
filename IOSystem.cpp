#include "IOSystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <cerrno>


int logFile;

void setupLogging(const char* logFilePath) {
    logFile = open(logFilePath, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (logFile < 0) {
        perror("Failed to open log file");
        exit(EXIT_FAILURE);
    }
}

void print(const char* format, ...) {
    va_list args;

    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    va_start(args, format);
    char buffer[MAX_BUFFER_SIZE];
    vsnprintf(buffer, MAX_BUFFER_SIZE, format, args);
    int e =  write(logFile, buffer, strlen(buffer));
    va_end(args);
}


void logToFileOnly(const char* format) {
    char buffer[MAX_BUFFER_SIZE];
    int size = strlen(format);
    memcpy(buffer, format, size);
    buffer[size] = '\r';
    buffer[size + 1] = '\n';
    int e = write(logFile, buffer, size + 2);
}

void printError(const char* msg) {
    char buffer[MAX_BUFFER_SIZE];
    snprintf(buffer, MAX_BUFFER_SIZE, "%s: %s\n", msg, strerror(errno));
    fputs(buffer, stderr);
    int e = write(logFile, buffer, strlen(buffer));
}