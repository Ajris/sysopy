#include <stdio.h>
#include <stdlib.h>

struct input{
    char* filename;
    int time;
    int type;
};

void printError(char* message);
struct input* parseArguments(char** argv);

int main(int argc, char **argv) {
    if(argc != 4){
        printError("Wrong number of arguments");
    }
    struct input* input = parseArguments(argv);

    printf("%s, %d, %d", input->filename, input->time, input->type);

    free(input);
}

struct input* parseArguments(char** argv){
    struct input* input = malloc(sizeof(struct input));
    input->filename = argv[1];
    input->time = atoi(argv[2]);
    input->type = atoi(argv[3]);
    return input;
}

void printError(char *message) {
    printf("%s\n", message);
    exit(1);
}