
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

    char date_res[MAX_LINE];
    char pipe_string[MAX_LINE];

    int count = atoi(argv[2]);

    printf("PID: %d\n", getpid());

    int pipe = open(argv[1], O_WRONLY);
    if	(pipe < 0)
        printError("Something went wrong");

    for (int i = 0; i < count; i++) {
        FILE *date = popen("date", "r");
        fgets(date_res, MAX_LINE, date);

        if (sprintf(pipe_string, "%d @ %s", (int) getpid(), date_res) < 0)
            printError("Could not write to string");

        if (write(pipe, pipe_string, strlen(pipe_string)) < 0)
            printError("Something went wrong");

        sleep(rand() % 4 + 2);
    }

    close(pipe);
    return 0;
}

void printError(char *message) {
    printf("ERROR: %s", message);
    exit(1);
}