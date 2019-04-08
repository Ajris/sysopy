
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <memory.h>

#define MAX_LINE 100

void printError(char *message);

int main(int argc, char **argv) {
    srand(time(NULL));
    if (argc != 3)
        printError("Wrong num of arguments");

    char *fileName = argv[1];
    char *dateRes = malloc(MAX_LINE * sizeof(char));
    char *wholeString = malloc(MAX_LINE * sizeof(char));

    int count = atoi(argv[2]);

    printf("PID: %d\n", getpid());

    int pipe = open(fileName, O_WRONLY);
    if (pipe < 0)
        printError("Something went wrong");

    for (int i = 0; i < count; i++) {
        FILE *date = popen("date", "r");
        fgets(dateRes, MAX_LINE, date);
        sprintf(wholeString, "PID: %d || DATE: %s", getpid(), dateRes);
        write(pipe, wholeString, strlen(wholeString));
        sleep(rand() % 4 + 2);
    }

    free(wholeString);
    free(dateRes);
    close(pipe);
    return 0;
}

void printError(char *message) {
    printf("ERROR: %s", message);
    exit(1);
}