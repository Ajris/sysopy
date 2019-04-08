#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <memory.h>
#include <sys/stat.h>

#define MAX_LINE 100

void printError(char *message);

int main(int argc, char **argv) {
    if (argc != 2)
        printError("Wrong num of arguments");

    char* fileName = argv[1];
    char* line = malloc(MAX_LINE * sizeof(char));

    mkfifo(fileName, S_IWUSR | S_IRUSR);
    FILE *file = fopen(fileName, "r");
    if (!file)
        printError("Couldnt open file");

    while (fgets(line, MAX_LINE, file) != NULL) {
        printf("=>%s", line);
    }

    free(line);
    fclose(file);
    return 0;
}

void printError(char *message) {
    printf("ERROR: %s", message);
    exit(1);
}