#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

void printError(char *message);

void generateFile(char *filename, int recordNumber, int recordSize);

char *generateRecord(int recordSize);

void sortFile(char *filename, int recordNumber, int recordSize, char *type);

void copyFile(char *from, char *to, int recordNumber, int recordSize, char *type);

int main(int argc, char **argv) {
    srand((unsigned int) time(NULL));
    if (argc == 1) {
        printError("Not enough parameters");
    }

    struct tms *begin = malloc(sizeof(struct tms));
    struct tms *end = malloc(sizeof(struct tms));


    for (int i = 1; i < argc; i++) {
        times(begin);
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
            if (i + 4 >= argc) {
                printError("Not enough parameters for sorting");
            }
            char *fileName = argv[i + 1];
            int recordNumber = atoi(argv[i + 2]);
            int recordSize = atoi(argv[i + 3]);
            char *type = argv[i + 4];

            sortFile(fileName, recordNumber, recordSize, type);

            i += 4;
        } else if (strcmp(argv[i], "copy") == 0) {
            if (i + 5 >= argc) {
                printError("Not enough parameters for copying");
            }

            char *from = argv[i + 1];
            char *to = argv[i + 2];
            int recordNumber = atoi(argv[i + 3]);
            int recordSize = atoi(argv[i + 4]);
            char *type = argv[i + 5];

            copyFile(from, to, recordNumber, recordSize, type);

            i += 5;
        } else {
            printError("Wrong argument");
        }
        times(end);
        printf("USER: %lf - SYSTEM: %lf\n",
               (double) (end->tms_utime - begin -> tms_utime) / sysconf(_SC_CLK_TCK),
               (double) (end->tms_stime - begin -> tms_stime) / sysconf(_SC_CLK_TCK));
    }
}

void copyFile(char *from, char *to, int recordNumber, int recordSize, char *type) {
    if (recordNumber <= 0) {
        printError("Wrong record number");
    }
    if (recordSize <= 0) {
        printError("Wrong record size");
    }

    unsigned char *buffer = malloc(recordSize * sizeof(char));

    if (strcmp(type, "sys") == 0) {
        int fromFile = open(from, O_RDONLY);
        int toFile = open(to, O_WRONLY);

        for (int i = 0; i < recordNumber; i++) {
            int elementsRead = read(fromFile, buffer, recordSize * sizeof(char));
            if (elementsRead != recordSize) {
                printf("%d,%d", elementsRead, recordSize);
                printError("Error reading");
            }

            write(toFile, buffer, recordSize * sizeof(char));
        }

        close(toFile);
        close(fromFile);
    } else if (strcmp(type, "lib") == 0) {
        FILE *fromFile = fopen(from, "r");
        if (!fromFile) {
            printError("Cannot open reading file");
        }
        FILE *toFile = fopen(to, "w");
        if (!toFile) {
            printError("Error in writing");
        }

        for (int i = 0; i < recordNumber; i++) {
            size_t elementsRead = fread(buffer, sizeof(char), recordSize, fromFile);
            if (elementsRead != recordSize) {
                printError("Error reading");
            }

            fwrite(buffer, sizeof(char), recordSize, toFile);
        }

        fclose(fromFile);
        fclose(toFile);
    } else {
        printError("Wrong type of sorting");
    }

    free(buffer);
}


void sortFile(char *filename, int recordNumber, int recordSize, char *type) {
    if (recordNumber <= 0) {
        printError("Wrong record number");
    }
    if (recordSize <= 0) {
        printError("Wrong record size");
    }

    unsigned char *firstBuffer = malloc(recordSize * sizeof(char));
    unsigned char *secondBuffer = malloc(recordSize * sizeof(char));

    if (strcmp(type, "sys") == 0) {
        int file = open(filename, O_RDWR);
        for (int i = 0; i < recordNumber - 1; i++) {
            lseek(file, i * recordSize, SEEK_SET);
            size_t elementsRead = (size_t) read(file, firstBuffer, recordSize * sizeof(char));
            if (elementsRead != recordSize) {
                printError("Error reading");
            }

            int currentMinimum = i;
            unsigned char firstBufChar = firstBuffer[0];

            for (int j = i + 1; j < recordNumber; j++) {
                lseek(file, j * recordSize, SEEK_SET);
                unsigned char secondBufChar;
                elementsRead = (size_t) read(file, &secondBufChar, sizeof(char));
                if (elementsRead != 1) {
                    printError("Error reading");
                }
                if (firstBufChar > secondBufChar) {
                    currentMinimum = j;
                    firstBufChar = secondBufChar;
                }
            }

            lseek(file, currentMinimum * recordSize, SEEK_SET);

            elementsRead = (size_t) read(file, secondBuffer, recordSize * sizeof(char));
            if (elementsRead != recordSize) {
                printError("Error reading");
            }

            lseek(file, currentMinimum * recordSize, SEEK_SET);

            write(file, firstBuffer, recordSize * sizeof(char));

            lseek(file, i * recordSize, SEEK_SET);

            write(file, secondBuffer, recordSize * sizeof(char));
        }
        close(file);
    } else if (strcmp(type, "lib") == 0) {
        FILE *file = fopen(filename, "r+");
        if (!file) {
            printError("Error opening file");
        }
        for (int i = 0; i < recordNumber - 1; i++) {
            fseek(file, i * recordSize, SEEK_SET);
            size_t elementsRead = fread(firstBuffer, sizeof(char), (size_t) recordSize, file);
            if (elementsRead != recordSize) {
                printError("Error reading");
            }

            int currentMinimum = i;
            unsigned char firstBufChar = firstBuffer[0];

            for (int j = i + 1; j < recordNumber; j++) {
                fseek(file, j * recordSize, SEEK_SET);
                elementsRead = fread(secondBuffer, sizeof(char), (size_t) recordSize, file);
                if (elementsRead != recordSize) {
                    printError("Error reading");
                }
                unsigned char secondBufChar = secondBuffer[0];
                if (firstBufChar > secondBufChar) {
                    currentMinimum = j;
                    firstBufChar = secondBufChar;
                }
            }

            fseek(file, currentMinimum * recordSize, SEEK_SET);

            elementsRead = fread(secondBuffer, sizeof(char), (size_t) recordSize, file);
            if (elementsRead != recordSize) {
                printError("Error reading");
            }

            fseek(file, currentMinimum * recordSize, SEEK_SET);

            fwrite(firstBuffer, sizeof(char), recordSize, file);

            fseek(file, i * recordSize, SEEK_SET);

            fwrite(secondBuffer, sizeof(char), recordSize, file);

            if (ferror(file))
                printError("Something went wrong with writing to file");

        }
        fclose(file);
    } else {
        printError("Wrong type of sorting");
    }

    free(firstBuffer);
    free(secondBuffer);
}

char *generateRecord(int size) {
    char *output = malloc(size * sizeof(char));
    for (int i = 0; i < size; i++) {
        output[i] = (char) (rand() % 256);
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
        fwrite(generateRecord(recordSize), (size_t) recordSize, 1, file);
    }

    fclose(file);
}

void printError(char *message) {
    printf("%s", message);
    exit(1);
}