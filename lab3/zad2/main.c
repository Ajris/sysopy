#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zconf.h>

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
};


void printError(char *message);

struct input *parseArguments(char **argv);

struct fileData **readFromFile(char *filename);

void createProcesses(struct fileData ** fileData, struct input* input);

int main(int argc, char **argv) {
    if (argc != 4) {
        printError("Wrong number of arguments");
    }
    struct input *input = parseArguments(argv);
    struct fileData **fileData = readFromFile(input->filename);
    createProcesses(fileData, input);

    printf("%s, %d, %d\n", input->filename, input->monitoringTime, input->type);

    for (int i = 0; i < MAX_FILE_NUM; i++) {
        free(fileData[i]);
    }
    free(fileData);
    free(input);
}

void createProcesses(struct fileData ** fileData, struct input* input){
    for (int i = 0; i < numOfFiles; i++) {
        if (vfork() == 0) {
            printf("%s-%d\n",fileData[i]->path,getpid());
            exit(1);
        }
    }
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
        char *splitedLine = strtok(currentLine, " ");
        fileData[numOfFiles]->path = splitedLine;
        splitedLine = strtok(NULL, " ");
        fileData[numOfFiles]->repeatTime = atoi(splitedLine);
        splitedLine = strtok(NULL, " ");

        if (splitedLine != NULL)
            printError("Wrong amount of arguments for file specified");

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
    input->monitoringTime = atoi(argv[2]);
    input->type = atoi(argv[3]);
    return input;
}

void printError(char *message) {
    printf("%s\n", message);
    exit(1);
}