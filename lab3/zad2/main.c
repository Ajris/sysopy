
#include <stdio.h>
#include <stdlib.h>

void printError(char* message);

int main(int argc, char **argv) {
    if(argc != 4){
        printError("Wrong number of arguments");
    }
    char* filename = argv[1];
    char* time = argv[2];
    char* type = argv[3];
    printf("%s %s %s", filename, time, type);
}

void printError(char *message) {
    printf("%s\n", message);
    exit(1);
}