#pragma once
#define MAX_BUFFER_SIZE 1024

void setupLogging(const char* logFilePath);
void print(const char* format, ...);
void logToFileOnly(const char* format);
void printError(const char* msg);