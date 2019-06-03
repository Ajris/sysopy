#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define KEY  "/home/ajris"

struct mess {
    long mtype;
    long val;
};

int main() {
    sleep(1);
    int val = 0;
    int id = msgget(ftok(KEY, 1), 0666);
    struct mess m;
    msgrcv(id, &m, sizeof(long), -100, 0);
    val = m.val;
    /**********************************
    Otworz kolejke systemu V "reprezentowana" przez KEY
    **********************************/
    /**********************************
    Odczytaj zapisane w kolejce wartosci i przypisz je do zmiennej val
    obowiazuja funkcje systemu V
    ************************************/

    printf("%d square is: %d\n", val, val * val);

    /*******************************
    posprzataj
    ********************************/
    msgctl(id, IPC_RMID, NULL);

    return 0;
}
