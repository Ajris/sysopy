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
    char line[MAX_LINE];

    if (mkfifo(fileName, S_IWUSR | S_IRUSR) < 0)
        printError("Wrong num of arguments");

    FILE *file = fopen(fileName, "r");
    if (!file)
        printError("Couldnt open file");

    while (fgets(line, MAX_LINE, file) != NULL) {
        if (write(1, line, strlen(line)) < 0)
            printError("Error");
    }

    fclose(file);
    return 0;
}

void printError(char *message) {
    printf("ERROR: %s", message);
    exit(1);
}