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
    createProcesses(fileData, input);

    for (int i = 0; i < MAX_FILE_NUM; i++) {
        free(fileData[i]);
    }
    free(fileData);
    free(input);
}

void createProcesses(struct fileData **fileData, struct input *input) {
    int *tmp = malloc(sizeof(int));
    time_t* startTime = malloc(sizeof(time_t) * numOfFiles);
    time_t* endTime = malloc(sizeof(time_t) * numOfFiles);

    for (int i = 0; i < numOfFiles; i++) {
        sleep(1);
        startTime[i] = time(NULL);
        endTime[i] = startTime[i] + input->monitoringTime;
        if ((fileData[i]->pid = fork()) == 0) {
            execl("watch", "-n", fileData[i]->repeatTime, "-d", "cat", fileData[i]->path, NULL);
            printf("%s-%d\n", fileData[i]->path, getpid());
            while(startTime[i] < endTime[i]){
                startTime[i] = time(NULL);
            }
            exit(11 + i);
        }
    }

    for (int i = 0; i < numOfFiles; i++) {
        waitpid(fileData[i]->pid, tmp, 0);
        printf("Process %d created %d file copies of %s.\n", fileData[i]->pid, WEXITSTATUS(tmp[0]), fileData[i]->path);
    }
    free(tmp);
}

struct fileData **readFromFile(char *filename) {
    FILE *file = fopen(filename, "r");
    struct fileData **fileData = malloc(sizeof(struct fileData *) * MAX_FILE_NUM);
    for (int i = 0; i < MAX_FILE_NUM; i++) {
        fileData[i] = malloc(sizeof(struct fileData) + sizeof(char) * MAX_FILELINE);
        fileData[i]->pid = 0;
    }

    if (!file)
        printError("Couldn't open file");

    char *currentLine = malloc(sizeof(char) * MAX_FILELINE);

    while (fgets(currentLine, MAX_FILELINE, file) != NULL) {
        char *token = strtok(currentLine, " ");
        fileData[numOfFiles]->path = strdup(token);
        char *ptr = strtok(NULL, " ");
        fileData[numOfFiles]->repeatTime = atoi(ptr);
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
    input->monitoringTime = atoi(argv[2]);
    input->type = atoi(argv[3]);
    return input;
}

void printError(char *message) {
    printf("%s\n", message);
    exit(1);
}