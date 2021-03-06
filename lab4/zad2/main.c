#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zconf.h>
#include <wait.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>

#define MAX_FILE_NUM 100
#define MAX_FILELINE 100

int numOfFiles = 0;
int isWorking = 1;
int isEnded = 0;
int justToWorkPlsDontDoIt = 0;

struct input {
    char *filename;
};

struct fileData {
    char *path;
    int pid;
    int stopped;
    int repeatingTime;
    int numOfCopies;
};

struct fileData **fileData;

void ctrlC_Handler(int signum);

void printError(char *message);

struct input *parseArguments(char **argv);

struct fileData **readLinesFromFile(char *filename);

void createProcesses(struct fileData **fileData);

int main(int argc, char **argv) {
    if (argc != 2) {
        printError("Wrong number of arguments");
    }

    struct sigaction act;
    act.sa_handler = ctrlC_Handler;
    sigemptyset(&act.sa_mask);
    sigaction(SIGINT, &act, NULL);

    struct input *input = parseArguments(argv);
    fileData = readLinesFromFile(input->filename);
    createProcesses(fileData);

    for (int i = 0; i < MAX_FILE_NUM; i++) {
        free(fileData[i]);
    }
    free(fileData);
    free(input);
}

char *getContent(char *filename) {
    struct stat fileinfo;
    if (lstat(filename, &fileinfo) != 0) {
        printError("Couldnt read file");
    }

    FILE *file = fopen(filename, "r");
    if (!file) {
        printError("Couldnt read file");
    }

    char *content = malloc(fileinfo.st_size + 1);
    if (fread(content, 1, fileinfo.st_size, file) != fileinfo.st_size) {
        printError("Could not read file");
    }

    content[fileinfo.st_size] = '\0';
    fclose(file);
    return content;
}

char *getOnlyFileName(char *filePath) {
    char *fileName = strchr(filePath, '/');
    if (fileName == NULL) {
        printError("Something went wrong with parsing name of file");
    }
    return fileName + 1;
}

void stop(int signum) {
    isWorking = 0;
}

void start(int signum) {
    isWorking = 1;
}

void end(int sig) {
    isEnded = 1;
}

void ctrlC_Handler(int signum) {
    if (signum == SIGINT)
        justToWorkPlsDontDoIt = 1;
}

void watchFileToMemory(struct fileData *fileData) {
    signal(SIGUSR1, &stop);
    signal(SIGUSR2, &start);
    signal(SIGRTMIN, &end);

    struct stat fileStats;
    if (lstat(fileData->path, &fileStats) == -1) {
        printf("File: %s", fileData->path);
        printError("Coudlnt read file");
    }

    time_t lastModification = fileStats.st_mtime;

    char *content = getContent(fileData->path);
    while (1) {
        if (isEnded == 1)
            break;
        while (isWorking == 1) {
            if (lstat(fileData->path, &fileStats) == -1) {
                printf("File: %s", fileData->path);
                printError("Coudlnt read file");
            }
            char *modificationTime = malloc(sizeof(char) * 1000);
            strftime(modificationTime, 1000, "_%Y-%m-%d_%H-%M-%S", localtime(&fileStats.st_mtime));
            char *newFileName = malloc(1000 * sizeof(char));
            sprintf(newFileName, "archiwum/%s%s", getOnlyFileName(fileData->path), modificationTime);
            if (lastModification < fileStats.st_mtime) {
                DIR *dir = opendir("archiwum");
                if (!dir) {
                    mkdir("archiwum", 0777);
                }
                FILE *file = fopen(newFileName, "w");
                if (!file) {
                    printError("Coudlnt created file");
                }
                fwrite(content, 1, strlen(content), file);
                fclose(file);
                lastModification = fileStats.st_mtime;
                content = getContent(fileData->path);
                fileData->numOfCopies++;
                closedir(dir);
            }
            free(modificationTime);
            free(newFileName);
            sleep(fileData->repeatingTime);
            if (isEnded == 1)
                break;
        }
        if (isWorking == 0)
            sleep(1);
    }
    free(content);
    exit(fileData->numOfCopies);
}


void monitorEverything(struct fileData **fileData) {
    while (1) {
        char *value = malloc(sizeof(char) * 64);
        fgets(value, 20, stdin);
        if (strcmp(value, "END\n") == 0 || justToWorkPlsDontDoIt == 1) {
            for (int i = 0; i < numOfFiles; i++) {
                int status;
                kill(fileData[i]->pid, SIGRTMIN);
                waitpid(fileData[i]->pid, &status, 0);
                printf("PROCESS: %d || FILE: %s || BACKUPS:%d\n", fileData[i]->pid, fileData[i]->path,
                       WEXITSTATUS(status));
            }
            free(value);
            exit(1);
        } else if (strcmp(value, "LIST\n") == 0) {
            for (int i = 0; i < numOfFiles; i++) {
                printf("PROCESS: %d || FILE: %s || IS STOPPED:%d\n", fileData[i]->pid, fileData[i]->path,
                       fileData[i]->stopped);
            }
        } else if (strcmp(value, "STOP ALL\n") == 0) {
            for (int i = 0; i < numOfFiles; i++) {
                kill(fileData[i]->pid, SIGUSR1);
                fileData[i]->stopped = 1;
            }
        } else if (strcmp(value, "START ALL\n") == 0) {
            for (int i = 0; i < numOfFiles; i++) {
                kill(fileData[i]->pid, SIGUSR2);
                fileData[i]->stopped = 0;
            }
        } else if (strncmp(value, "STOP", 4) == 0) {
            strtok(value, " ");
            char *ptr = strtok(NULL, " ");
            if (ptr != NULL) {
                int pid = atoi(ptr);
                int found = 0;
                for (int i = 0; i < numOfFiles; i++) {
                    if (fileData[i]->pid == pid) {
                        fileData[i]->stopped = 1;
                        kill(pid, SIGUSR1);
                        found = 1;
                    }
                }
                if (!found) {
                    printf("Couldn't find that pid\n");
                }
            } else {
                printf("You didnt specified pid\n");
            }
        } else if (strncmp(value, "START", 5) == 0) {
            strtok(value, " ");
            char *ptr = strtok(NULL, " ");
            if (ptr != NULL) {
                int pid = atoi(ptr);
                int found = 0;
                for (int i = 0; i < numOfFiles; i++) {
                    if (fileData[i]->pid == pid) {
                        fileData[i]->stopped = 0;
                        kill(pid, SIGUSR2);
                        found = 1;
                    }
                }
                if (!found) {
                    printf("Couldn't find that pid\n");
                }
            } else {
                printf("You didnt specified pid\n");
            }
        } else {
            printf("Have no idea what to do\n");
        }
        free(value);
    }
}


void createProcesses(struct fileData **fileData) {
    for (int i = 0; i < numOfFiles; i++) {
        pid_t curr = fork();
        if (curr == 0) {
            watchFileToMemory(fileData[i]);
        } else {
            fileData[i]->pid = curr;
        }
    }
    monitorEverything(fileData);
}

struct fileData **readLinesFromFile(char *filename) {
    FILE *file = fopen(fileName, "r");
    struct fileData **fileData = malloc(sizeof(struct fileData *) * MAX_FILE_NUM);
    for (int i = 0; i < MAX_FILE_NUM; i++) {
        fileData[i] = malloc(sizeof(struct fileData) + 2 * sizeof(int) + sizeof(char) * MAX_FILELINE);
    }

    if (!file)
        printError("Couldn't open file");

    char *currentLine = malloc(sizeof(char) * MAX_FILELINE);

    while (fgets(currentLine, MAX_FILELINE, file) != NULL) {
        char *token = strtok(currentLine, " ");
        fileData[numOfFiles]->path = strdup(token);
        char *ptr = strtok(NULL, " ");
        fileData[numOfFiles]->repeatingTime = atoi(ptr);
        ptr = strtok(NULL, " ");
        if (ptr != NULL)
            printError("Wrong number of arguments near file");
        numOfFiles++;

        if (numOfFiles > MAX_FILE_NUM)
            printError("Too many files added in lista");
    }

    fclose(file);
    return fileData;
}

struct input *parseArguments(char **argv) {
    struct input *input = malloc(sizeof(struct input));
    input->filename = argv[1];
    return input;
}

void printError(char *message) {
    printf("%s\n", message);
    exit(0);
}