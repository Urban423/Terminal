#include "Terminal.h"
#include "IOSystem.h"

int main() {
    setupLogging("./log/log.txt");
    Terminal terminal;
    while(terminal.isRunning()) {
        terminal.processCommand();
    }

    return 0;
}