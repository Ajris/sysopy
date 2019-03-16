#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>

void printError(char *message);

void generateFile(char *filename, int recordNumber, int recordSize);

char *generateRecord(int recordSize);

int main(int argc, char **argv) {
    if (argc == 1) {
        printError("Not enough parameters");
    }
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "generate") == 0) {
            if (i + 3 >= argc) {
                printError("Not enough parameters for generate");
            }
            char *fileName = argv[i + 1];
            int recordNumber = atoi(argv[i + 2]);
            int recordSize = atoi(argv[i + 3]);
            i += 3;
            generateFile(fileName, recordNumber, recordSize);
        } else if (strcmp(argv[i], "sort") == 0) {

        } else if (strcmp(argv[i], "copy") == 0) {

        } else {
            printError("Wrong argument");
        }
    }
}

char *generateRecord(int size) {
    char *output = malloc(size * sizeof(char));
    for (int i = 0; i < size; i++) {
        output[i] = (char) ((char) rand() % 256);
    }
    return output;
}

void generateFile(char *filename, int recordNumber, int recordSize) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        printError("Error opening file");
    }
    if (recordNumber <= 0) {
        printError("Wrong record number");
    }
    if (recordSize <= 0) {
        printError("Wrong record size");
    }

    for (int i = 0; i < recordNumber; i++) {
        fprintf(file, "%s", generateRecord(recordSize));
    }

    fclose(file);
}

void printError(char *message) {
    printf("%s", message);
    exit(1);
}