#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_PARAMS 10
#define MAX_COMMANDS 10
#define MAX_COMMAND_LENGTH 40
#define MAX_LINE_LENGTH 200

void printError(char *message);

int getNumOfLines(char *wholeFile);

char **getLines(char *wholeFile, int numOfLines);

int main(int argc, char **argv) {
    if (argc != 2)
        printError("Wrong number of parameters");
    FILE *file = fopen(argv[1], "r");
    if (file == NULL)
        printError("Couldn't open file");

    fseek(file, 0, SEEK_END);
    size_t fileLength = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *wholeFile = malloc(fileLength + 1);

    if (fread(wholeFile, 1, fileLength, file) != fileLength)
        printError("Error while reading file");

    printf("%s\n", wholeFile);

    int numOfLines = getNumOfLines(wholeFile);

    printf("%s\n", wholeFile);

    printf("%d\n", numOfLines);
    char **lines = getLines(wholeFile, numOfLines);

    for (int i = 0; i < numOfLines; i++) {
        printf("LINE %d:%s\n", i, lines[i]);
    }

    fclose(file);
}

char **getLines(char *wholeFile, int numOfLines) {
    char **lines = malloc(numOfLines * sizeof(char *));
    for (int i = 0; i < numOfLines; i++) {
        lines[i] = malloc(MAX_LINE_LENGTH * sizeof(char));
    }

    char *copy = malloc(strlen(wholeFile) * sizeof(char));
    strcpy(copy, wholeFile);
    lines[0] = strtok(copy, "\n");
    int i = 1;
    while ((lines[i++] = strtok(NULL, "\n"))) {

    }
    free(copy);
    return lines;
}

int getNumOfLines(char *wholeFile) {
    int noOfTokens = 1;
    char *copy = malloc(strlen(wholeFile) * sizeof(char));
    strcpy(copy, wholeFile);
    strdup(copy);
    strtok(copy, "\n");
    while (strtok(NULL, "\n")) {
        noOfTokens++;
    }
    free(copy);
    return noOfTokens;
}

void printError(char *message) {
    printf("ERROR: %s", message);
    exit(1);
}

