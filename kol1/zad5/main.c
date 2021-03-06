#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>


int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Not a suitable number of program parameters\n");
        exit(1);
    }

    int toChildFD[2];
    int toParentFD[2];

    pipe(toChildFD);
    pipe(toParentFD);

    int val1, val2, val3 = 0;

    pid_t child;

    if ((child = fork()) == 0) {
        close(toChildFD[1]);
        char* readingBuffer = malloc(sizeof(char) * 100);
        read(toChildFD[0], readingBuffer, 99);
        char* error;
        val2 = strtol(readingBuffer, &error, 10);
        if(strcmp(error, "") != 0){
            printf("SOMETHING WENT WRONG\n");
            exit(1);
        }
        //odczytaj z potoku nienazwanego wartosc przekazana przez proces macierzysty i zapisz w zmiennej val2

        val2 = val2 * val2;
        char* writingBuffer = malloc(sizeof(char) * 100);
        sprintf(writingBuffer, "%d", val2);
        close(toParentFD[0]);
        //wyslij potokiem nienazwanym val2 do procesu macierzysego
        write(toParentFD[1], writingBuffer, 99);
        close(toChildFD[0]);
        close(toParentFD[1]);
    } else {
        close(toChildFD[0]);
        write(toChildFD[1], argv[1], 10);
        //wyslij val1 potokiem nienazwanym do priocesu potomnego

        sleep(1);
        close(toParentFD[1]);
        char* readingBuffer = malloc(100 * sizeof(char));
        read(toParentFD[0], readingBuffer, 99);
        val3 = atoi(readingBuffer);
        close(toChildFD[1]);
        close(toParentFD[0]);
        //odczytaj z potoku nienazwanego wartosc przekazana przez proces potomny i zapisz w zmiennej val3
        printf("Square is: %d\n", val3);
    }
    return 0;
}
