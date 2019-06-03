#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#define KEY "/home/ajris"

struct mess{
    long mtype;
    long value;
};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Not a suitable number of program parameters\n");
        return (1);
    }
    /******************************************************
    Utworz kolejke komunikatow systemu V "reprezentowana" przez KEY
    Wyslij do niej wartosc przekazana jako parametr wywolania programu
    Obowiazuja funkcje systemu V
    ******************************************************/
    key_t key = ftok(KEY, 0);
    int val = atoi(argv[1]);
    int queueID = msgget(key, 0666 | IPC_CREAT);
    struct mess buf;
    buf.mtype = 1;
    buf.value = val;
    if (msgsnd(queueID, &buf, sizeof(long), 0) < 0) {
        perror("msgsnd");
    }
    return 0;
}