#include "Terminal.h"
#include "IOSystem.h"

int main(int argc, char** argv) {
    setupLogging("/home/urban/Desktop/Dop/Terminal/log/log.txt");

    
    Terminal terminal(argc, argv);



    while(terminal.isRunning()) {
        terminal.processCommand();
    }

    return 0;
}