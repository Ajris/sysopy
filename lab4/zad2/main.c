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

struct input {
    char *filename;
};

struct fileData {
    char *path;
};

void printError(char *message);

struct input *parseArguments(char **argv);

struct fileData **readFromFile(char *filename);

void createProcesses(struct fileData **fileData, struct input *input);

int main(int argc, char **argv) {
    if (argc != 2) {
        printError("Wrong number of arguments");
    }
    struct input *input = parseArguments(argv);
    struct fileData **fileData = readFromFile(input->filename);
    createProcesses(fileData, input);

    for (int i = 0; i < MAX_FILE_NUM; i++) {
        free(fileData[i]);
    }
    free(fileData);
    free(input);
}

void watchFileToMemory(struct fileData *fileData, struct input *input) {
    int numOfCopies = 0;

    struct stat fileStats;
    if (lstat(fileData->path, &fileStats) == -1) {
        printf("File: %s", fileData->path);
        printError("Coudlnt read file");
    }

    while (1) {
        if (lstat(fileData->path, &fileStats) == -1) {
            printf("File: %s", fileData->path);
            printError("Coudlnt read file");
        }
    }
    exit(numOfCopies);
}

void createProcesses(struct fileData **fileData, struct input *input) {
    for (int i = 0; i < numOfFiles; i++) {
        pid_t curr = fork();
        if (curr == 0)
            watchFileToMemory(fileData[i], input);
        else {
        }
    }
    int *tmp = malloc(sizeof(int));

    for (int i = 0; i < numOfFiles; i++) {
        pid_t curr = wait(tmp);
        printf("Process %d created %d file copies.\n", curr, WEXITSTATUS(tmp[0]));
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