#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_PARAMS 10
#define MAX_COMMANDS 10
#define MAX_LINE_LENGTH 200

int numOfLines = 0;

char** getCommandElements(char *line);

int getNumberOfTokens(char* string, char* tokens);

char ** readFromFile(char* fileName);

void printError(char *message);

void execLine(char* line);

int main(int argc, char **argv) {
    if (argc != 2)
        printError("Wrong number of parameters");

    char** lines = readFromFile(argv[1]);

    for(int i = 0; i < numOfLines; i++)
        execLine(lines[i]);
}

char** readFromFile(char* fileName){
    FILE *file = fopen(fileName, "r");
    if (file == NULL)
        printError("Couldn't open file");

    char *currentLine = malloc(sizeof(char) * MAX_LINE_LENGTH);

    char **lines = malloc(sizeof(char *) * MAX_COMMANDS * sizeof(char) * MAX_LINE_LENGTH);

    while (fgets(currentLine, MAX_LINE_LENGTH, file) != NULL) {
        char *token = strtok(currentLine, "\n");
        lines[numOfLines] = strdup(token);
        numOfLines++;
        if (numOfLines > MAX_COMMANDS)
            printError("Too many commands");
    }
    fclose(file);
    return lines;
}

int getNumberOfTokens(char* string, char* tokens) {
    int noOfTokens = 1;
    char* stringCpy = calloc(strlen(string), sizeof(char));
    strcpy(stringCpy, string);

    strtok(stringCpy, tokens);
    while(strtok(NULL, tokens)) {
        noOfTokens++;
    }

    free(stringCpy);

    return noOfTokens;
}

char** getCommandElements(char *line){
    char tokens[3] = {' ','\n','\t'};
    int noOfArguments = getNumberOfTokens(line, tokens);

    char** arguments = calloc(noOfArguments+1, sizeof(char*));
    arguments[0] = strtok(line, tokens);

    for(int i=1; i<noOfArguments; ++i) {
        arguments[i] = strtok(NULL, tokens);
    }

    arguments[noOfArguments] = NULL;

    return arguments;
}

void execLine(char* line) {
    int commandsNumber = 1;
    int pipes[2][2];
    char *commands[MAX_COMMANDS];

    char* lineCpy = calloc(strlen(line), sizeof(char));
    strcpy(lineCpy, line);

    strtok(lineCpy, "|");
    while(strtok(NULL, "|")) {
        commandsNumber++;
    }

    free(lineCpy);

    commands[0] = strtok(line, "|");
    for(int i=1; i<commandsNumber; ++i) {
        commands[i] = strtok(NULL, "|");
    }

    for (int i=0; i<commandsNumber; ++i) {
        if (i>0) {
            close(pipes[i % 2][0]);
            close(pipes[i % 2][1]);
        }

        if(pipe(pipes[i % 2]) == -1) {
            printf("Błąd potoku\n");
            exit(-1);
        }

        if (fork() == 0) {
            char** commandElements = getCommandElements(commands[i]);

            if (i != commandsNumber-1) {
                close(pipes[i % 2][0]);
                if (dup2(pipes[i % 2][1], STDOUT_FILENO) < 0) {
                    exit(-1);
                }
            }
            if (i != 0) {
                close(pipes[(i + 1) % 2][1]);
                if (dup2(pipes[(i + 1) % 2][0], STDIN_FILENO) < 0) {
                    close(-1);
                }
            }
            execvp(commandElements[0], commandElements);
            free(commandElements);

            exit(0);
        }
    }
    close(pipes[commandsNumber % 2][0]);
    close(pipes[commandsNumber % 2][1]);
    close(pipes[(commandsNumber + 1) % 2][0]);
    close(pipes[(commandsNumber + 1) % 2][1]);
    for(int i=0; i<commandsNumber; ++i) {
        wait(NULL);
    }
}

void printError(char *message) {
    printf("ERROR: %s", message);
    exit(1);
}