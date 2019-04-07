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

int getNumOfLines(char *string);

int main(int argc, char **argv) {
    if (argc != 2)
        printError("Wrong number of parameters");
    FILE *file = fopen(argv[1], "r");
    if (file == NULL)
        printError("Couldn't open file");

    fseek(file, 0 , SEEK_END);
    size_t fileLength = ftell(file);
    fseek(file, 0 , SEEK_SET);

    char *wholeFile = malloc(fileLength + 1);

    if(fread(wholeFile, 1, fileLength, file) != fileLength)
        printError("Error while reading file");

    printf("%s\n", wholeFile);

    int numOfLines = getNumOfLines(wholeFile);
    printf("%d", numOfLines);

    fclose(file);
}

int getNumOfLines(char *string) {
    int noOfTokens = 1;
    char* stringCpy = malloc(strlen(string) * sizeof(char));
    strcpy(stringCpy, string);
    strtok(stringCpy, "\n");
    while(strtok(NULL, "\n")) {
        noOfTokens++;
    }
    free(stringCpy);
    return noOfTokens;
}

void printError(char *message) {
    printf("ERROR: %s", message);
    exit(1);
}

