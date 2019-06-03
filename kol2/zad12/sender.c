#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define KEY "/home/ajris"

struct mess{
    long mtype;
    long val;
};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Not a suitable number of program parameters\n");
        return (1);
    }
    int id = msgget(ftok(KEY, 1), IPC_CREAT | 0666);
    struct mess m;
    m.mtype = 1;
    m.val = atoi(argv[1]);
    msgsnd(id, &m, sizeof(long), 0);
    /******************************************************
    Utworz kolejke komunikatow systemu V "reprezentowana" przez KEY
    Wyslij do niej wartosc przekazana jako parametr wywolania programu
    Obowiazuja funkcje systemu V
    ******************************************************/


    return 0;
}



