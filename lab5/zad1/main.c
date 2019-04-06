
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

void printError(char* message);

int main(int argc, char **argv) {
    if(argc != 2)
        printError("Wrong number of parameters");
    char* file = argv[1];
    printf("%s", file);
}

void printError(char* message){
    printf("ERROR: %s", message);
    exit(1);
}

