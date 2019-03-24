//
// Created by ajris on 24.03.19.
//
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <zconf.h>
#include <wait.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>

void printError(char *message);

int main(int argc, char **argv) {
    if (argc != 5)
        printError("Wrong number of arguments");

    char *fileName = argv[1];
    int pmin = atoi(argv[2]);
    int pmax = atoi(argv[3]);
    int bytes = atoi(argv[4]);

    if(pmin <= 0 || pmax <= 0 || bytes <= 0 || pmin > pmax){
        printError("Wrong input");
    }

    srand(time(NULL));

    char *additionalContent = malloc(bytes + 1);

    for (int i = 0; i < bytes; i++) {
        additionalContent[i] = (rand() % 57) + 65;
    }

    char *date = malloc(50);

    while (1) {
        int wait = rand() % (abs(pmax - pmin) + 1) + pmin;
        sleep(wait);
        FILE *file = fopen(fileName, "a");
        time_t t = time(NULL);
        strftime(date, 50, "_%Y-%m-%d_%H-%M-%S", localtime(&t));
        fprintf(file, "PID: %d SECONDS: %d ACTUALDATE: %s CONTENT: %s\n",getpid(), wait, date, additionalContent);
        fclose(file);
    }
}


void printError(char *message) {
    printf("%s\n", message);
    exit(1);
}