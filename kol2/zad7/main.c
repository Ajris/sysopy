#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/wait.h>
#include <semaphore.h>

#define FILE_NAME "common.txt"
#define SEM_NAME "my_kol_sem"

int main(int argc, char **args) {
    if (argc != 4) {
        printf("Not a suitable number of program parameters\n");
        return (1);
    }
    /************************************
    Utworz semafor posixowy. Ustaw jego wartosc na 1
    *************************************/
    sem_t *sem_id = sem_open(SEM_NAME, O_CREAT, 0600, 1);
    int fd = open(FILE_NAME, O_WRONLY | O_CREAT , 0644);

    int parentLoopCounter = atoi(args[1]);
    int childLoopCounter = atoi(args[2]);

    char buf[50];
    pid_t childPid;
    int max_sleep_time = atoi(args[3]);
    if ((childPid = fork())) {
        int status = 0;
        srand((unsigned) time(0));

        while (parentLoopCounter--) {
            int s = rand() % max_sleep_time + 1;
            sleep(s);
            /**************q****************************
            Sekcja krytyczna. Zabezpiecz dostep semaforem
            ******************************************/
            sem_wait(sem_id);
            sprintf(buf, "Wpis rodzica. Petla %d. Spalem %d\n", parentLoopCounter, s);
            write(fd, buf, strlen(buf));
            write(1, buf, strlen(buf));
            /****************************************************
            Koniec sekcji krytycznej
            ****************************************************/
            sem_post(sem_id);
        }
        waitpid(childPid, &status, 0);
    } else {
        srand((unsigned) time(0));
        while (childLoopCounter--) {
            int s = rand() % max_sleep_time + 1;
            sleep(s);
            /******************************************
            Sekcja krytyczna. Zabezpiecz dostep semaforem
            ******************************************/
            sem_wait(sem_id);
            sprintf(buf, "Wpis dziecka. Petla %d. Spalem %d\n", childLoopCounter, s);
            write(fd, buf, strlen(buf));
            write(1, buf, strlen(buf));
            /****************************************************
            Koniec sekcji krytycznej
            ****************************************************/
            sem_post(sem_id);
        }
        _exit(0);
    }
    /***************************************
    Posprzataj semafor
    ***************************************/
    sem_close(sem_id);
    sem_unlink(SEM_NAME);
    close(fd);
    return 0;
}