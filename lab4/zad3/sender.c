
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

void printError(char *message);

int main(int argc, char **argv) {
    if (argc != 4)
        printError("Wrong number of arguments");

    int numOfSignals = atoi(argv[1]);
    int catcherPID = atoi(argv[2]);
    char *mode = argv[3];

    if(strcmp(mode, "KILL") == 0){

    } else if(strcmp(mode, "SIGQUEUE") == 0){

    } else if(strcmp(mode, "SIGRT") == 0){

    } else {
        printError("wrong mode");
    }
}

void printError(char *message) {
    printf("%s", message);
    exit(1);
}