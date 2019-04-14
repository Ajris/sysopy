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
        char* buffer = malloc(sizeof(char) * 100);
        read(toChildFD[0], buffer, 99);
        val2 = atoi(buffer);
        //odczytaj z potoku nienazwanego wartosc przekazana przez proces macierzysty i zapisz w zmiennej val2

        val2 = val2 * val2;
        char* buff = malloc(sizeof(char) * 100);
        sprintf(buff, "%d", val2);
        printf("%s\n", buff);
        //wyslij potokiem nienazwanym val2 do procesu macierzysego
        dup2(toChildFD[1], STDOUT_FILENO);
        write(toChildFD[1], buff, 99);

    } else {
        close(toParentFD[0]);
        val1 = atoi(argv[1]);
        dup2(toParentFD[1], STDOUT_FILENO);
        write(toParentFD[1], argv[1], 10);
        //wyslij val1 potokiem nienazwanym do priocesu potomnego

        sleep(1);

        char* buffer = malloc(100 * sizeof(char));
        read(toChildFD[0], buffer, 99);
        val3 = atoi(buffer);
        //odczytaj z potoku nienazwanego wartosc przekazana przez proces potomny i zapisz w zmiennej val3

        printf("%d square is: %d\n", val1, val3);
    }
    return 0;
}
