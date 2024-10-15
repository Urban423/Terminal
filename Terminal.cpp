#include "Terminal.h"
#include "IOSystem.h"
#include <string.h>
#include <stdlib.h>
#include <dirent.h> 
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h> 
#include <fcntl.h>
#include <stdio.h>

Terminal* terminal = nullptr;

void handleSigint(int sig) {
    terminal->killAll();
}


void custom_ls(const char *path, StringBuffer* tipsBuffer, bool silent) {
    struct dirent *entry;
    DIR *dir = opendir(path);
    if (dir == NULL) {
        printError("Unable to open directory");
        return;
    }

    tipsBuffer->offset_size = 0;
    tipsBuffer->size = 0;
    char tempFileName[MAX_FILE_NAME_LENGTH];
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        strncpy(tempFileName, entry->d_name, MAX_FILE_NAME_LENGTH - 1);
        tempFileName[MAX_FILE_NAME_LENGTH - 1] = '\0';

        int fileNameLength = strlen(tempFileName);
        tipsBuffer->offset[tipsBuffer->offset_size] = tipsBuffer->size;
        strcpy(tipsBuffer->array + tipsBuffer->size, tempFileName);

        tipsBuffer->size += fileNameLength + 1; 
        tipsBuffer->offset_size++;
        if(silent) {continue;}
        print("\t%s\n", tempFileName);
    }

    closedir(dir);
}

char custom_cd(const char* path) {
	char cwd[PATH_MAX];

	if (chdir(path) != 0) {
		printError("cd failed");
	}

    char* e = getcwd(cwd, sizeof(cwd));
	return 0;
}

void custom_cat(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    int ch;
    while ((ch = fgetc(file)) != EOF) { 
        putchar(ch);
    }
	printf("\n");
    fclose(file); 
}

void killProcess(pid_t pid, int signal, int* pids, int& size) {
    int found = 0;
    for (int i = 0; i < size; i++) {
        if (pids[i] == pid) {
            found = 1;
            for (int j = i; j < size - 1; j++) {
                pids[j] = pids[j + 1];
            }
            size--;  
            break;
        }
    }

    if (!found) {
        printf("PID: %d not found in the array.\n", pid);
        return;
    }


    if (kill(pid, signal) == -1) {
        printError("Error sending signal");
        exit(EXIT_FAILURE);
    } else {
        print("Successfully sent signal %d to process %d\n", signal, pid);
    }
}

int changeProcessPriority(pid_t pid, int niceValue) {
    if (niceValue < -20 || niceValue > 19) {
        fprintf(stderr, "Nice value must be between -20 (highest priority) and 19 (lowest priority).\n");
        return -1;
    }

    if (setpriority(PRIO_PROCESS, pid, niceValue) == -1) {
        printError("Failed to change process priority");
        return -1;
    }

    print("Changed priority of process %d to %d\n", pid, niceValue);
    return 0;
}







int executeProcess(int argc, char** argv) {
    char processName[MAX_FILE_NAME_LENGTH];
    char filename[MAX_FILE_NAME_LENGTH];
    int isBackground = 0;
    if (argv[0][0] == '&') {
        isBackground = 1;
        ++argv[0];
        if(argv[0][0] == 0) {
            argc--;
            ++argv;
        }
    }

    if (argv[0][0] != '/' && strncmp(argv[0], "./", 2) != 0) {
        snprintf(processName, MAX_FILE_NAME_LENGTH, "./%s", argv[0]);
        argv[0] = strdup(processName); 
    }
    print("%s\n", argv[0]);
    
    pid_t pid = vfork(); 
    if (pid < 0) {
        printError("vfork failed");
        return -1;
    } 
    else if (pid == 0) {
        argv[argc] = NULL; 

        if (isBackground) {
            snprintf(filename, MAX_FILE_NAME_LENGTH, "./log/%d.txt", getpid());
            int devNull = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (devNull < 0) {
                printError("open /dev/null failed");
                return -1;
            }
            dup2(devNull, STDOUT_FILENO);
            dup2(devNull, STDERR_FILENO);
            close(devNull);
        }
        execvp(argv[0], argv);
        printError("execvp failed");
        return -1;
    } 
    else {
        if (isBackground) {
            print("Parent process: Child process running in background (PID: %d)\n", pid);
        } else {
            wait(NULL);
            print("Parent process: Child process finished\n");
            return -1;
        }
        return pid;
    }
    return -1;
}





Terminal::Terminal() 
{
    size = 0;
    is_running = true;
    tipsBuffer = line.getTipsBuffer();
    custom_ls("./", tipsBuffer, true);
    signal(SIGINT, handleSigint);

    terminal = this;
}

Terminal::~Terminal() 
{

}




void Terminal::processCommand() 
{
    int signal = SIGTERM;
    processInput in = line.getProccesInput();
    if(in.ArgC == 0) { return; }


    //for(int i = 0; i < in.ArgC; i++) {
    //    printf("%s\n", in.Args[i]);
    //}

    if(strcmp(in.Args[0], "ls") == 0) {
        if(in.ArgC == 2) {
            custom_ls(in.Args[1], tipsBuffer, false);
            return;
        }
        custom_ls("./", tipsBuffer, false);
    }
    else if(strcmp(in.Args[0], "cat")== 0) {
        if(in.ArgC  == 1) {
            print("\tUsage: cat <filepath>\n");
            return;
        }
        custom_cat(in.Args[1]);
    }
    else if(strcmp(in.Args[0], "nice") == 0) {
        if(size == 0) {return;}
        changeProcessPriority(ids[0], 0);
    }
    else if(strcmp(in.Args[0], "cd") == 0) {
        if(in.ArgC  == 1) {
            print("\tUsage: cd <path>\n");
            return;
        }
        custom_cd(in.Args[1]);
        //custom_ls("./", tipsBuffer, true);
    }
    else if(in.ArgC == 2 && strcmp(in.Args[0], "kill") == 0) {
        if(strcmp(in.Args[1], "all") == 0) {
            killAll();
        }
        else {
            int id = atoi(in.Args[1]);
            killProcess(id, signal, ids, size);
        }
    }
    else if(strcmp(in.Args[0], "ps") == 0) {
        for(int i = 0; i < size; i++) {
            print("%d) %d\n", i, ids[i]);
        }
    }
    else {
        int id = executeProcess(in.ArgC, in.Args);
        if(id != -1) {
            ids[size] = id;
            size++;
        }
    }
}

bool Terminal::isRunning() {
    return is_running;
}

void Terminal::killAll() {
    for(int i = 0; i < size; i++) {
        print("\nCaught Ctrl+C. Terminating child process (PID: %d)...\n", ids[i]);
        kill(ids[i], SIGKILL); 
    }
    size = 0;
}