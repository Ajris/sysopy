#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zconf.h>
#include <wait.h>
#include <time.h>

#define MAX_FILE_NUM 100
#define MAX_FILELINE 100

int numOfFiles = 0;

struct input {
    char *filename;
    int monitoringTime;
    int type;
};

struct fileData {
    char *path;
    int repeatTime;
    pid_t pid;
};


void printError(char *message);

struct input *parseArguments(char **argv);

struct fileData **readFromFile(char *filename);

void createProcesses(struct fileData **fileData, struct input *input);

int main(int argc, char **argv) {
    if (argc != 4) {
        printError("Wrong number of arguments");
    }
    struct input *input = parseArguments(argv);
    struct fileData **fileData = readFromFile(input->filename);
//    createProcesses(fileData, input);

    for (int i = 0; i < MAX_FILE_NUM; i++) {
        free(fileData[i]);
    }
    free(fileData);
    free(input);
}

void createProcesses(struct fileData **fileData, struct input *input) {
    int *tmp = malloc(sizeof(int));
    time_t start = time(NULL);
    time_t currentTime = start;
    for (int i = 0; i < numOfFiles; i++) {
        if ((fileData[i]->pid = fork()) == 0) {
//            execl("watch", "-n", fileData[i]->repeatTime,"-d", "cat", fileData[i]->path, NULL);
            printf("%s-%d\n", fileData[i]->path, getpid());
//            if(start >= currentTime + 10){
            exit(11 + i);
//            }
        }
    }

    for (int i = 0; i < numOfFiles; i++) {
        wait(tmp);
        printf("Process %d created %d file copies of %s.\n", fileData[i]->pid, WEXITSTATUS(tmp[0]), fileData[i]->path);
    }

    free(tmp);
}

struct fileData **readFromFile(char *filename) {
    FILE *file = fopen(filename, "r");
    struct fileData **fileData = malloc(sizeof(struct fileData *) * MAX_FILE_NUM);
    for (int i = 0; i < MAX_FILE_NUM; i++) {
        fileData[i] = malloc(sizeof(struct fileData) + sizeof(char) * MAX_FILELINE);
    }

    if (!file)
        printError("Couldn't open file");

    char *currentLine = malloc(sizeof(char) * MAX_FILELINE);

    while (fgets(currentLine, MAX_FILELINE, file) != NULL) {
        char *token = strtok(currentLine, " ");
        fileData[numOfFiles]->path = strdup(token);
        char* ptr = strtok(NULL, " ");
        fileData[numOfFiles]->repeatTime = atoi(ptr);
        ptr = strtok(NULL, " ");
        if(ptr != NULL)
            printError("Wrong number of arguments near file");
        numOfFiles++;

        if (numOfFiles > MAX_FILE_NUM)
            printError("Too many files added in lista");
    }

    for (int i = 0; i < numOfFiles; i++) {
        printf("%s-%d\n", fileData[i]->path, fileData[i]->repeatTime);
    }

    fclose(file);
    return fileData;
}

struct input *parseArguments(char **argv) {
    struct input *input = malloc(sizeof(struct input));
    input->filename = argv[1];
    input->monitoringTime = atoi(argv[2]);
    input->type = atoi(argv[3]);
    return input;
}

void printError(char *message) {
    printf("%s\n", message);
    exit(1);
}