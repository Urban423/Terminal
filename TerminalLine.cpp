#include "TerminalLine.h"
#include "IOSystem.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <string.h>


//terminal 
inline void disableEchoMode() 
{
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt); 
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

inline void restoreTerminalMode() {
    struct termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    oldt.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}


inline int getLastWord(char** ptr, BufferLine& line) {
    int i = line.size - 1;

    while (i >= 0 && line.buffer[i] != ' ') {
        i--;
    }
    *ptr = line.buffer + i + 1;

    return line.size - i - 1;
}

//input handle
inline int splitInput(char* inputBuffer, char* (*inputArgs)[BUFFER_SIZE]) {
	int count_words = 0;
	char* ptr = inputBuffer;
	bool flag = false;
	while(1) {
		if(*ptr == 0) 	{ break; }
		if(*ptr == ' ') { flag = false; *ptr = 0;}
		else {
			if(flag == false) {
				(*inputArgs)[count_words] = ptr;
				count_words++;
			}
			flag = true;
		}
		ptr++;
	}
	return count_words;
}

inline void processArrowKeys(BufferLine (*lines)[BUFFER_COUNT], int buffersCount, int& bufferIndex, int& lineIndex) {
    char seq[2];
    seq[0] = getchar();
    seq[1] = getchar(); 
	
    if (seq[0] != '[') {
        return;
    }
    switch (seq[1]) {
        case 'A': //up
            char before;
            if(buffersCount >= BUFFER_COUNT) {
                before =  (bufferIndex + BUFFER_COUNT - 1) % BUFFER_COUNT;
            }else if(bufferIndex - 1 > -1) {
                before = bufferIndex -1;
            }
            bufferIndex = before;

          //  printf(" %d %d   ", bufferIndex, buffersCount);
            lineIndex = (*lines)[bufferIndex].size;
            printf("\33[2K\r%s%s", INPUT_SPEACH, (*lines)[bufferIndex].buffer);
            break;
        case 'B': //down
            char after;
            if(buffersCount >= BUFFER_COUNT) {
                after =  (bufferIndex + 1) % BUFFER_COUNT;
            }else if(bufferIndex < buffersCount) {
                after = bufferIndex + 1;
            }
            bufferIndex = after;

            //printf("   %d %d   ", bufferIndex, buffersCount);
            lineIndex = (*lines)[bufferIndex].size;
            printf("\33[2K\r%s%s", INPUT_SPEACH, (*lines)[bufferIndex].buffer);
            break;
        case 'C': //right
           // printf("%d", (*lines)[bufferIndex].size);
            if(lineIndex >= (*lines)[bufferIndex].size) {return;}
            lineIndex++;
            printf("\x1b[C");
            break;
        case 'D': //left
            if(lineIndex == 0) { return; }
            lineIndex--;
            printf("\x1b[D");
            break;
    }
}

inline void processCharacter(BufferLine& line, char c, int& lineIndex) {
    if(line.size == lineIndex) {
        line.buffer[lineIndex] = c;
        lineIndex++;
        line.size++;
        printf("%c", c);
        return;
    }

    printf("%c%s", c, line.buffer + lineIndex);
    char* ptr =  line.buffer + line.size - 1;
    for(int i = 0; i < line.size - lineIndex; i++){
        printf("\x1b[D");
        *(ptr + 1) = *(ptr);
        ptr--;
    }
    line.buffer[lineIndex] = c;
    line.buffer[line.size + 1] = 0;
    //printf("\n%s\n", line.buffer);
    lineIndex++;
    line.size++;
}

inline void processBackspace(BufferLine& line, int& lineIndex) {
    if(lineIndex == 0) {return;}

    line.buffer[line.size] = 0;
    printf("\b%s ", line.buffer + lineIndex);
    char* ptr =  line.buffer + lineIndex - 1;
    for(int i = 0; i < line.size - lineIndex + 1; i++){
        printf("\x1b[D");
        *(ptr++) = *(ptr + 1);
    }
    line.buffer[line.size - 1] = 0;
    //printf("\n%s\n", line.buffer);


    lineIndex--;
    line.size--;
}

inline void processTab(StringBuffer* tipsBuffer, BufferLine& line, int& lineIndex) {
    char* lastWord;
    int lastWordLen = getLastWord(&lastWord, line);
    //printf("%s  %d\n", lastWord, lastWordLen);
    if (lastWord[0] == '\0') {
        return;
    }

    for (int i = 0; i < tipsBuffer->offset_size; i++) {
        const char* tip = tipsBuffer->array + tipsBuffer->offset[i];
        //printf("%s\n", tip);
        if (strncmp(lastWord, tip, lastWordLen) == 0) {
            strcpy((char*)lastWord, tip);
            int tipLen = strlen(tip);
            line.size = (lastWord - line.buffer) + tipLen;
            lineIndex += tipLen - lastWordLen;

            printf("%s", tip + lastWordLen);
            return;
        }
    }
}




TerminalLine:: TerminalLine() { 
    tipsBuffer.array = (char*)malloc(MAX_FILES * MAX_FILE_NAME_LENGTH * sizeof(char));
    tipsBuffer.offset = (int*)malloc(MAX_FILES * sizeof(int));
}

TerminalLine::~TerminalLine() { }

processInput TerminalLine::getProccesInput()
{
    lineIndex = 0;

    disableEchoMode();
	printf(INPUT_SPEACH);
    while(1) 
    {
        int c = getchar();
        if(c == BACK_SPACE_KEYCODE) {
           processBackspace(lines[bufferIndex], lineIndex);
        } 
        else if(c == '\x1b') {
            processArrowKeys(&lines, buffersCount, bufferIndex, lineIndex);
        }
        else if(c == '\n') {
            printf("\n");
            break;
        }
        else if(c == '\t') {
            processTab(&tipsBuffer, lines[bufferIndex], lineIndex);
            continue;
        }
        else {
            processCharacter(lines[bufferIndex], c, lineIndex);
        }
	}
    restoreTerminalMode();



    logToFileOnly(lines[bufferIndex].buffer);
    int mustIndex = buffersCount % BUFFER_COUNT;
    if(mustIndex != bufferIndex) {
        memcpy(lines[mustIndex].buffer, lines[bufferIndex].buffer, lines[bufferIndex].size);
        lines[mustIndex].size = lines[bufferIndex].size;
    }
    memcpy(resultBuffer, lines[bufferIndex].buffer, lines[bufferIndex].size);
    resultBuffer[lines[bufferIndex].size] = '\0';
    //for(int i = 0; i < lines[bufferIndex].size; i++) { printf("%c\n", resultBuffer[i]);   }
    //printf("%s\n", resultBuffer);
    processInput result;
    result.ArgC = splitInput(resultBuffer, &result.Args);



    bufferIndex = (++buffersCount) % BUFFER_COUNT;
    if(lines[bufferIndex].size != 0) {
        lines[bufferIndex].size = 0;
        memset(lines[bufferIndex].buffer, 0, BUFFER_SIZE);
    }
    return result;
}

StringBuffer* TerminalLine::getTipsBuffer() 
{
    return &tipsBuffer;
}