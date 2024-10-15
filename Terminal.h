#pragma once
#include "TerminalLine.h"

#define PATH_MAX    4096
#define ID_MAX      100

class Terminal
{
public:
     Terminal();
    ~Terminal();

    void killAll();
    bool isRunning();
    void processCommand();
private:
    TerminalLine line;
    bool is_running;

    StringBuffer* tipsBuffer;
    int ids[ID_MAX];
    int size; 
};