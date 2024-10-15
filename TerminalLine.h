#pragma once
#define BUFFER_SIZE 		    1024
#define BUFFER_COUNT 		    10
#define BACK_SPACE_KEYCODE 	    127
#define MAX_FILE_NAME_LENGTH    256
#define MAX_FILES               1000
#define INPUT_SPEACH            "speak> "
#define DEFAULT_SOLIDIUS        '/'

struct BufferLine {
    int     size = 0;
    char    buffer[BUFFER_SIZE];
};

struct processInput {
    char* 	Args[BUFFER_SIZE];
	int		ArgC;
};

struct StringBuffer {
    char* array;
    int* offset;
    int size;
    int offset_size;
};

class TerminalLine {
public:
     TerminalLine();
    ~TerminalLine();

    StringBuffer* getTipsBuffer();
    processInput getProccesInput();
private:
    BufferLine lines[BUFFER_COUNT];
	int buffersCount = 0;
	int bufferIndex = 0;
    int lineIndex = 0;

    char resultBuffer[BUFFER_SIZE];
    StringBuffer tipsBuffer;
};